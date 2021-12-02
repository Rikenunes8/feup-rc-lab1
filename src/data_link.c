#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "data_link.h"
#include "alarm.h"
#include "data_link_aux.h"
#include "log.h"
#include "macros_dl.h"


extern int finish;
extern int send_frame;
extern int n_sends;

static struct termios oldtio;
static LinkLayer ll;


int ll_open_transmitter(int fd) {
  uchar wframe[SU_SIZE];
  uchar rframe[SU_SIZE];
  uchar wanted_controls[] = {UA};
  const int N_CONTROLS = 1;
  
  create_su_frame(wframe, A_1, SET);

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  while (!finish) {
    if (send_frame) {
      if (write_frame(fd, wframe, SU_SIZE) < 0)
        log_err("Fail sending SET frame");
      else
        log_sent("SET", -1);

      alarm(ll.timeout);
      send_frame = FALSE;
    }

    if (read_su_frame(fd, A_1, wanted_controls, N_CONTROLS, rframe) == 0) {
      alarm(0);
      log_rcvd("UA", -1);
      finish = TRUE; 
    }
    else {
      log_err("Fail reading UA frame");
      if (n_sends >= ll.numTransmissions) {
        log_msg("Limit of resends reached");
        return -1;
      }
    }
  }

  return 0;
}

int ll_open_receiver(int fd) {
  uchar rframe[SU_SIZE];
  uchar wframe[SU_SIZE];
  uchar wanted_controls[] = {SET};
  const int N_CONTROLS = 1;

  if (read_su_frame(fd, A_1, wanted_controls, N_CONTROLS, rframe) < 0) {
    log_err("Fail reading SET frame");
    return -1;
  }
  log_rcvd("SET", -1);

  create_su_frame(wframe, A_1, UA);

  if (write_frame(fd, wframe, SU_SIZE) < 0) {
    log_err("Fail sending UA frame");
    return -1;
  }
  log_sent("UA", -1);
  
  return 0;
}

int llopen(char* port, int status) {
  int fd = open_non_canonical(port, &oldtio, 0, 0);
  if (fd < 0) {
    log_err("Openning port");
    return -1;
  }
  if (set_alarm() < 0) {
    log_err("Setting alarm");
    close_non_canonical(fd, &oldtio);
    return -1;
  }

  
  strcpy(ll.port, port);
  ll.baudRate = BAUDRATE;
  ll.sequenceNumber = 0x00;
  ll.timeout = TIME_OUT;
  ll.numTransmissions = MAX_RESENDS;
  

  int res;
  if (status == TRANSMITTER) {
    res = ll_open_transmitter(fd);
  }
  else if (status == RECEIVER) {
    res = ll_open_receiver(fd);
  }
  else {
    log_err("Wrong status");
    close_non_canonical(fd, &oldtio);
    return -1;
  }

  if (res < 0) {
    close_non_canonical(fd, &oldtio);
    return -1;
  }
  
  return fd;
}

int llwrite(int fd, uchar* data, int length) {
  if (fd < 0) {
    log_err("File is close");
    return -1;
  }
  uchar wframe[MAX_STUF_FRAME_SIZE];
  uchar rframe[SU_SIZE];
  
  const int N_CONTROLS = 2;
  uchar wanted_controls[N_CONTROLS];
  uchar control_to_send;

  if (ll.sequenceNumber%2 == 0) {
    control_to_send = S_0;
    wanted_controls[0] = RR_1;
    wanted_controls[1] = REJ_0;
  }
  else if (ll.sequenceNumber%2 == 1) {
    control_to_send = S_1;
    wanted_controls[0] = RR_0;
    wanted_controls[1] = REJ_1;
  }

  int frame_size = create_info_frame(wframe, control_to_send, data, length);
  frame_size = byte_stuffing(wframe, frame_size);
  
  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  while (!finish) {
    if (send_frame) {
      if (write_frame(fd, wframe, frame_size) < 0) {
        log_err("Fail sending INFO frame");
        return -1;
      }
      log_sent("INFO", ll.sequenceNumber);

      alarm(ll.timeout);
      send_frame = FALSE;
    }

    int res = read_su_frame(fd, A_1, wanted_controls, N_CONTROLS, rframe);
    if (res == 0) {
      alarm(0);
      finish = TRUE; 
      
      ll.sequenceNumber = (ll.sequenceNumber+1)%2;
      log_rcvd("RR", ll.sequenceNumber);
    }
    else if (res == 1) {
      alarm(0);
      send_frame = TRUE;
      n_sends = 0;
      log_rcvd("REJ", ll.sequenceNumber);
    }
    else {
      log_err("Fail reading RR/REJ frame");
      if (n_sends >= ll.numTransmissions) {
        log_msg("Limit of resends reached");
        return -1;
      }
    }
  }
  
  return 0;
}

int llread(int fd, uchar* buffer) {
  if (fd < 0) {
    log_err("File is close");
    return -1;
  }  
  uchar wframe[SU_SIZE];
  uchar rframe[MAX_STUF_FRAME_SIZE];
  
  uchar wanted_controls[] = {S_0, S_1};
  const int N_CONTROLS = 2;
  uchar index_control_rcvd; // Received control byte index
  uchar controls_to_send[] = {RR_0, RR_1, REJ_0, REJ_1};
  uchar index_control_to_send; // Control byte to be send index
  
  int frame_size;

  do {
    frame_size = read_info_frame(fd, A_1, wanted_controls, N_CONTROLS, rframe);
    frame_size = byte_destuffing(rframe, frame_size);

    if      (rframe[CNTRL_BYTE] == S_0) index_control_rcvd = 0;
    else if (rframe[CNTRL_BYTE] == S_1) index_control_rcvd = 1;

    log_rcvd("INFO", index_control_rcvd);
    
    if (index_control_rcvd == ll.sequenceNumber) { // If new frame
      if (rframe[frame_size-2] == BCC_2(rframe+DATA_BEGIN, frame_size-6)) { // If bcc2 is correct put data in buffer, choose a RR to be send and update sequence number
        memcpy(buffer, &rframe[DATA_BEGIN], frame_size-6);

        index_control_to_send = (index_control_rcvd+1)%2;
        ll.sequenceNumber = index_control_to_send;
      }
      else {  // Else keep sequence number and choose a REJ to be send
        index_control_to_send = 2+index_control_rcvd;
        ll.sequenceNumber = index_control_rcvd;
      }
    }
    else { // If frame was previously received successfully -> Discard data, choose a RR to be send and update sequence number
      index_control_to_send = (index_control_rcvd+1)%2;
      ll.sequenceNumber = index_control_to_send;
    }

    create_su_frame(wframe, A_1, controls_to_send[index_control_to_send]);
    if (write_frame(fd, wframe, SU_SIZE) < 0) {
      log_err("Fail sending RR/REJ frame");
      return -1;
    }
    
    char* text[] = {"RR", "REJ"};
    log_sent(text[index_control_to_send/2], index_control_to_send%2);
    
  } while (index_control_to_send / 2 != 0); // While the control byte send is not a RR
  return frame_size-6;
}

int ll_close_transmitter(int fd) {
  uchar wframe[SU_SIZE];
  uchar rframe[SU_SIZE];
  uchar wanted_controls[] = {DISC};
  const int N_CONTROLS = 1;
  
  create_su_frame(wframe, A_1, DISC);

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  while (!finish) {
    if (send_frame) {
      if (write_frame(fd, wframe, SU_SIZE) < 0)
        log_err("Fail sending DISC frame");
      else
        log_sent("DISC", -1);

      alarm(ll.timeout);
      send_frame = FALSE;
    }

    if (read_su_frame(fd, A_2, wanted_controls, N_CONTROLS, rframe) >= 0) {
      log_rcvd("DISC", -1);

      create_su_frame(wframe, A_2, UA);
      if (write_frame(fd , wframe, SU_SIZE) < 0)
        log_err("Fail sending UA frame");
      else
        log_sent("UA", -1);

      alarm(0);
      finish = TRUE; 
    }
    else {
      log_err("Fail reading DISC frame");
      if (n_sends >= ll.numTransmissions) {
        log_msg("Limit of resends reached");
        return -1;
      }
    }
  }
  return 0;
}

int ll_close_receiver(int fd) {
  uchar wframe[SU_SIZE];
  uchar rframe[SU_SIZE];
  uchar wanted_controls[] = {DISC};
  const unsigned int N_CONTROLS = 1;

  if (read_su_frame(fd, A_1, wanted_controls, N_CONTROLS, rframe) < 0) {
    log_err("Fail reading DISC frame");
    return -1;
  }
  log_rcvd("DISC", -1);

  create_su_frame(wframe, A_2, DISC);
  
  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;
  
  while (!finish) {
    if (send_frame) {
      if (write_frame(fd, wframe, SU_SIZE) < 0)
        log_err("Fail sending UA frame");
      else
        log_sent("DISC", -1);

      alarm(ll.timeout);
      send_frame = FALSE;
    }

    wanted_controls[0] = UA;    
    if (read_su_frame(fd, A_2, wanted_controls, N_CONTROLS, rframe) >= 0) {
      alarm(0);
      log_rcvd("UA", -1);
      finish = TRUE; 
    }
    else {
      log_err("Fail reading UA frame");
      if (n_sends >= ll.numTransmissions) {
        log_msg("Limit of resends reached");
        return -1;
      }
    }
  }
  return 0;
}

int llclose(int fd, int status) {
  if (fd < 0) {
    return -1;
  }

  int res;
  if (status == TRANSMITTER)
    res = ll_close_transmitter(fd);
  else if (status == RECEIVER)
    res = ll_close_receiver(fd);    

  close_non_canonical(fd, &oldtio);
  return res;
}

