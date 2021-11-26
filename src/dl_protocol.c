#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dl_protocol.h"
#include "sp_config.h"
#include "alarm.h"
#include "state_machine.h"
#include "auxiliar.h"
#include "log.h"


extern int finish;
extern int send_frame;
extern int n_sends;

static struct termios oldtio;
static LinkLayer ll;
//static unsigned sequence_number;



void create_su_frame(uchar* frame, uchar address, uchar control) {
  frame[0] = FLAG;
  frame[1] = address;
  frame[2] = control;
  frame[3] = get_BCC_1(frame[1], frame[2]);
  frame[4] = FLAG;
}

int create_info_frame(uchar* frame, uchar control, uchar* data, int data_length) {
  frame[0] = FLAG;
  frame[1] = A_1;
  frame[2] = control;
  frame[3] = get_BCC_1(frame[1], frame[2]);
  for (int i = 0; i < data_length; i++) {
    frame[4+i] = data[i];
  }
  frame[4+data_length] = get_BCC_2(data, data_length);
  frame[4+data_length+1] = FLAG;

  return 4+data_length+2;
}

int read_su_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame) {
  State_machine* sm = create_sm(address, controls, n_controls);
  uchar byte;
  while (sm->state != STOP && !finish && !send_frame) {
    if (read(fd, &byte, sizeof(char)) > 0) {
      event_handler_sm(sm, byte, frame, SUPERVISION);
    }
  }
  int control_chosen = sm->control_chosen;
  destroy_sm(sm);

  if (finish || send_frame)
    return -1;
  return control_chosen;
}

int read_info_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame) {
  State_machine* sm = create_sm(address, controls, n_controls);
  uchar byte;
  while (sm->state != STOP ) {
    if (read(fd, &byte, sizeof(char)) > 0) {
      event_handler_sm(sm, byte, frame, INFORMATION);
    }
  }
  
  int frame_size = sm->frame_size;
  destroy_sm(sm);

  return frame_size;
}

int write_frame(int fd, uchar* frame, unsigned size) {
  return write(fd, frame, size);
}




int ll_open_transmitter(int fd) {
  uchar wframe[SU_SIZE];
  uchar rframe[SU_SIZE];
  uchar controls[] = {UA};
  const int N_CONTROLS = 1;
  
  create_su_frame(wframe, A_1, SET);

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      if (write_frame(fd, wframe, SU_SIZE) < 0) {
        log_err("Fail reading SET frame");
        return -1;
      }
      log_sent("SET", -1);

      alarm(ll.timeout);
      send_frame = FALSE;
    }
    read_value = read_su_frame(fd, A_1, controls, N_CONTROLS, rframe);
    
    if (read_value >= 0) {
      alarm(0);
      finish = TRUE; 
    }
    else {
      log_err("Fail reading UA frame");
      if (n_sends >= ll.numTransmissions) {
        finish = TRUE;
      }
    }
  }

  if (read_value < 0) {
    log_msg("Limit of resends reached");
    return -1;
  }
  log_rcvd("UA", -1);

  return fd;
}

int ll_open_receiver(int fd) {
  uchar rframe[SU_SIZE];
  uchar wframe[SU_SIZE];
  uchar controls[] = {SET};
  const int N_CONTROLS = 1;
  if (read_su_frame(fd, A_1, controls, N_CONTROLS, rframe) < 0) {
    log_err("Fail reading SET frame");
    return -1;
  }
  log_rcvd("SET", -1);


  create_su_frame(wframe, A_1, UA);
  //create_su_frame(wframe, A_1, SET); // Test failed answer

  if (write_frame(fd, wframe, SU_SIZE) < 0) {
    log_err("Fail sending UA frame");
    return -1;
  }
  log_sent("UA", -1);
  return 0;
}

int llopen(char* port, int status) {

  int fd = open_non_canonical(port, &oldtio, 0, 5);
  if (fd < 0) {
    log_err("Openning port");
    return -1;
  }
  if (set_alarm() < 0) {
    log_err("Setting alarm");
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
    return -1;
  }

  if (res < 0) {
    log_err("Opening connection");
    close_non_canonical(fd, &oldtio);
    return -1;
  }
  
  return fd;
}

int llwrite(int fd, uchar* data, int length) {
  if (fd < 0) return -1;
  uchar wframe[MAX_STUF_FRAME_SIZE];
  uchar rframe[SU_SIZE];
  
  const int N_CONTROLS = 2;
  uchar controls[N_CONTROLS];

  uchar control_to_send;

  if (ll.sequenceNumber == 0) {
    control_to_send = S_0;
    controls[0] = RR_1;
    controls[1] = REJ_0;
  }
  else if (ll.sequenceNumber == 1) {
    control_to_send = S_1;
    controls[0] = RR_0;
    controls[1] = REJ_1;
  }
  else {
    return -1;
  }
  int frame_size = create_info_frame(wframe, control_to_send, data, length);
  frame_size = byteStuffing(wframe, frame_size);
  
  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, wframe, frame_size);
      log_sent("INFO", ll.sequenceNumber);

      alarm(ll.timeout);
      send_frame = FALSE;
    }

    read_value = read_su_frame(fd, A_1, controls, N_CONTROLS, rframe);

    if (read_value == 0) {
      alarm(0);
      finish = TRUE; 
      
      ll.sequenceNumber = (ll.sequenceNumber+1)%2;
      log_rcvd("RR", ll.sequenceNumber);
    }
    else if (read_value > 0) {
      alarm(0);
      send_frame = TRUE;
      n_sends = 0;
      log_rcvd("REJ", ll.sequenceNumber);
    }
    else if (n_sends >= ll.numTransmissions) {
      log_msg("Limit of resends");
      finish = TRUE;
    }
  }

  if (read_value < 0) {
    return -1;
  }
  
  return 0;
}

int llread(int fd, uchar* buffer) {
  if (fd < 0) return -1;
  uchar wframe[SU_SIZE];
  uchar rframe[MAX_STUF_FRAME_SIZE];
  
  uchar controls[] = {S_0, S_1};
  const int N_CONTROLS = 2;
  uchar controls_to_send[] = {RR_0, RR_1, REJ_0, REJ_1};
  int frame_size;
  int successfull_read = FALSE;
  // int test = 1;

  do {
    frame_size = read_info_frame(fd, A_1, controls, N_CONTROLS, rframe);
    frame_size = byteDestuffing(rframe, frame_size);
    // print_frame(rframe, frame_size);

    uchar control_used;
    if      (rframe[CNTRL_BYTE] == S_0) control_used = 0;
    else if (rframe[CNTRL_BYTE] == S_1) control_used = 1;
    else return -1;

    log_rcvd("INFO", control_used);
    /*if (test) {
      rframe[frame_size-2] = 0x33; // TEST
      test = 0;
    }*/
    
    uchar i_control;
    if (control_used == ll.sequenceNumber) { // new frame
      if (rframe[frame_size-2] == get_BCC_2(rframe+DATA_BEGIN, frame_size-6)) { // bcc2 is correct
        for (int i = 0; i < frame_size - 6; i++) {
          buffer[i] = rframe[DATA_BEGIN + i];
        }

        i_control = (control_used+1)%2;
        ll.sequenceNumber = i_control;
      }
      else {
        i_control = 2+control_used;
        ll.sequenceNumber = control_used;
      }
    }
    else { // same frame -> discard
      i_control = (control_used+1)%2;
      ll.sequenceNumber = i_control;
    }
    create_su_frame(wframe, A_1, controls_to_send[i_control]);

    write_frame(fd, wframe, SU_SIZE);
    
    char* text[] = {"RR", "REJ"};
    log_sent(text[i_control/2], i_control%2);
    
    if (i_control / 2 == 0)
      successfull_read = TRUE;
  } while (!successfull_read);
  return frame_size-6;
}

int ll_close_transmitter(int fd) {
  uchar wframe[SU_SIZE];
  uchar rframe[SU_SIZE];
  uchar controls[] = {DISC};
  const int N_CONTROLS = 1;
  
  create_su_frame(wframe, A_1, DISC);

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, wframe, SU_SIZE);
      log_sent("DISC", -1);

      alarm(ll.timeout);
      send_frame = FALSE;
    }
    read_value = read_su_frame(fd, A_2, controls, N_CONTROLS, rframe);


    
    if (read_value >= 0) {
      log_rcvd("DISC", -1);

      create_su_frame(wframe, A_2, UA); //check if this is ok pls 
      write_frame(fd , wframe, SU_SIZE);
      log_sent("UA", -1);

      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= ll.numTransmissions) {
      log_msg("Limit of resends reached");
      finish = TRUE;
    }
  }
  return 0;
}

int ll_close_receiver(int fd) {
  uchar wframe[SU_SIZE];
  uchar rframe[SU_SIZE];
  uchar controls[] = {DISC};
  const unsigned int N_CONTROLS = 1;
  if (read_su_frame(fd, A_1, controls, N_CONTROLS, rframe) < 0) {
    log_err("Reading DISC frame");
    return -1;
  }
  log_rcvd("DISC", -1);

  create_su_frame(wframe, A_2, DISC);
  
  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;
  
  int read_value;
  while (!finish) {
    if (send_frame) {
      if (write_frame(fd, wframe, SU_SIZE) < 0) {
        log_err("Sending UA frame");
        return -1; // TODO: should be this here?
      }
      log_sent("DISC", -1);

      alarm(ll.timeout);
      send_frame = FALSE;
    }
    controls[0] = UA;
    read_value = read_su_frame(fd, A_2, controls, N_CONTROLS, rframe);
    
    if (read_value >= 0) {
      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= ll.numTransmissions) {
      finish = TRUE;
    }
  }

  if (read_value < 0) {
    log_msg("Limit of resends reached");
    return -1;
  }
  log_rcvd("UA", -1);
  return 0;
}

int llclose(int fd, int status) {
  if (fd < 0) {
    return -1;
  }

  if (status == TRANSMITTER) {
    ll_close_transmitter(fd);
  }
  else if (status == RECEIVER) {
    ll_close_receiver(fd);
  }

  close_non_canonical(fd, &oldtio);
  return 0;
}

