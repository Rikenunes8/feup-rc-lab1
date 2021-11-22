#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dl_protocol.h"
#include "sp_config.h"
#include "alarm.h"
#include "state_machine.h"
#include "aux.h"


extern int finish;
extern int send_frame;
extern int n_sends;

static struct termios oldtio;
static unsigned sequence_number;



int create_su_frame(uchar* frame, uchar control, int who) {
  int is_command = control == SET || control == DISC;
  int is_answer = control == UA || control == RR_0 || control == RR_1 || control == REJ_0 || control == REJ_1;

  frame[0] = FLAG;
  if ((is_command && who == TRANSMITTER) || (is_answer && who == RECEIVER))
    frame[1] = A_1;
  else if ((is_command && who == RECEIVER) || (is_answer && who == TRANSMITTER))
    frame[1] = A_2;
  else {
    return -1;
  }
  frame[2] = control;
  frame[3] = get_BCC_1(frame[1], frame[2]);
  frame[4] = FLAG;

  return 0;
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
  else
    return control_chosen;
}

int read_info_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame) {
  uchar control;

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
  uchar frame_to_send[MAX_SIZE];
  uchar frame_to_receive[MAX_SIZE];
  uchar controls[] = {UA};
  const int N_CONTROLS = 1;
  
  if (create_su_frame(frame_to_send, SET, TRANSMITTER) < 0) {
    return -1;
  }

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, frame_to_send, SU_SIZE);
      printf("SET frame sent\n");

      alarm(TIME_OUT);
      send_frame = FALSE;
    }
    read_value = read_su_frame(fd, A_1, controls, N_CONTROLS, frame_to_receive);
    
    if (read_value >= 0) {
      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= MAX_RESENDS) {
      printf("Limit of resends\n");
      finish = TRUE;
    }
  }

  if (read_value < 0) {
    return -1;
  }
  printf("UA frame received\n");

  return fd;
}

int ll_open_receiver(int fd) {
  uchar frame_to_receive[MAX_SIZE];
  uchar controls[] = {SET};
  const unsigned int N_CONTROLS = 1;
  int res = read_su_frame(fd, A_1, controls, N_CONTROLS, frame_to_receive);
  printf("SET frame received\n");


  uchar frame_to_send[MAX_SIZE];
  create_su_frame(frame_to_send, UA, RECEIVER);
  // create_su_frame(frame_to_send, SET, RECEIVER); // Test failed answer

  write_frame(fd, frame_to_send, SU_SIZE);
  printf("UA frame sent\n");

  return 0;
}

int llopen(char* port, int who) {

  int fd = open_non_canonical(port, &oldtio, 0, 5);
  if (fd < 0) {
    return -1;
  }
  set_alarm();

  sequence_number = 0;


  if (who == TRANSMITTER) {
    int ok = ll_open_transmitter(fd);
    if (ok < 0) {
      close_non_canonical(fd, &oldtio);
      return -1;
    }
    else {
      return fd;
    }
  }
  else if (who == RECEIVER) {
    int ok = ll_open_receiver(fd);
    if (ok < 0) {
      close_non_canonical(fd, &oldtio);
      return -1;
    }
    else {
      return fd;
    }
  }
  return -1;
}

int llwrite(int fd, uchar* data, int length) {
  uchar frame_to_send[MAX_SIZE];
  uchar frame_to_receive[MAX_SIZE];
  //int len_stuffed = byteStuffing(data, length);
  const int N_CONTROLS = 2;
  uchar controls[N_CONTROLS];

  uchar control_to_send;

  if (sequence_number == 0) {
    control_to_send = S_0;
    controls[0] = RR_1;
    controls[1] = REJ_0;
  }
  else if (sequence_number == 1) {
    control_to_send = S_1;
    controls[0] = RR_0;
    controls[1] = REJ_1;
  }
  else {
    return -1;
  }
  int frame_size = create_info_frame(frame_to_send, control_to_send, data, length);

  
  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      //write_frame(fd, buffer, len_stuffed);
      write_frame(fd, frame_to_send, frame_size);
      printf("INFORMATION Ns%d frame sent\n", sequence_number);

      alarm(TIME_OUT);
      send_frame = FALSE;
    }

    read_value = read_su_frame(fd, A_1, controls, N_CONTROLS, frame_to_receive);

    if (read_value >= 0) {
      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= MAX_RESENDS) {
      printf("Limit of resends\n");
      finish = TRUE;
    }
  }

  if (read_value < 0) {
    return -1;
  }
  else if (read_value == 0) {
    sequence_number = (sequence_number+1)%2;
    printf("RR_%d frame received\n", sequence_number);
  }
  else {
    printf("REJ_%d frame received\n", sequence_number);
  }
}

int llread(int fd, uchar* buffer) {
  uchar frame_to_send[MAX_SIZE];
  uchar frame_to_receive[MAX_SIZE];
  
  uchar controls[] = {S_0, S_1};
  const int N_CONTROLS = 1;
  int frame_size = read_info_frame(fd, A_1, controls, N_CONTROLS, frame_to_receive);
  
  uchar control_used;
  if      (frame_to_receive[CNTRL_BYTE] == S_0) control_used = 0;
  else if (frame_to_receive[CNTRL_BYTE] == S_1) control_used = 1;
  else return -1;

  printf("INFORMATION Ns%d frame received\n", control_used);
  
  uchar controls_to_send[] = {RR_0, RR_1, REJ_0, REJ_1};
  
  uchar i_control;
  if (control_used == sequence_number) { // new frame
    if (frame_to_receive[frame_size-2] == get_BCC_2(frame_to_receive+DATA_BEGIN, frame_size-6)) { // bcc2 is correct
      for (int i = 0; i < frame_size - 6; i++) {
        buffer[i] = frame_to_receive[DATA_BEGIN + i];
      }

      i_control = (control_used+1)%2;
      sequence_number = i_control;
    }
    else {
      i_control = 2+control_used;
      sequence_number = control_used;
    }
  }
  else { // same frame -> discard
    i_control = (control_used+1)%2;
    sequence_number = i_control;
  }
  create_su_frame(frame_to_send, controls_to_send[i_control], RECEIVER);

  write_frame(fd, frame_to_send, SU_SIZE);
  
  char* text[] = {"RR_0", "RR_1", "REJ_0", "REJ_1"};
  printf("%s frame sent\n", text[i_control]);

  return frame_size-6;
}

int ll_close_transmitter(int fd) {
  uchar frame_to_send[MAX_SIZE];
  uchar frame_to_receive[MAX_SIZE];
  uchar controls[] = {DISC};
  const int N_CONTROLS = 1;
  
  if (create_su_frame(frame_to_send, DISC, TRANSMITTER) < 0) {
    return -1;
  }

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, frame_to_send, SU_SIZE);
      printf("DISC frame sent\n");

      alarm(TIME_OUT);
      send_frame = FALSE;
    }
    read_value = read_su_frame(fd, A_2, controls, N_CONTROLS, frame_to_receive);


    
    if (read_value >= 0) {
      printf("DISC frame received\n");

      create_su_frame(frame_to_send, UA, TRANSMITTER); //check if this is ok pls 
      write_frame(fd , frame_to_send, SU_SIZE);
      printf("UA frame sent\n");

      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= MAX_RESENDS) {
      printf("Limit of resends\n");
      finish = TRUE;
    }
  }
  return 0;
}

int ll_close_receiver(int fd) {
  uchar frame_to_receive[MAX_SIZE];
  uchar controls[] = {DISC};
  const unsigned int N_CONTROLS = 1;
  int res = read_su_frame(fd, A_1, controls, N_CONTROLS, frame_to_receive);
  printf("DISC frame received\n");

  char frame_to_send[MAX_SIZE];
  if (create_su_frame(frame_to_send, DISC, RECEIVER) < 0) {
    return -1;
  }
  
  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;
  
  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, frame_to_send, SU_SIZE);
      printf("DISC frame sent\n");

      alarm(TIME_OUT);
      send_frame = FALSE;
    }
    controls[0] = UA;
    read_value = read_su_frame(fd, A_2, controls, N_CONTROLS, frame_to_receive);
    
    if (read_value >= 0) {
      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= MAX_RESENDS) {
      printf("Limit of resends\n");
      finish = TRUE;
    }
  }

  if (read_value < 0) {
    return -1;
  }
  
  printf("UA frame received\n");
  return 0;
}

int llclose(int fd, int who) {
  if (fd < 0) {
    return -1;
  }

  if (who == TRANSMITTER) {
    ll_close_transmitter(fd);
  }
  else if (who == RECEIVER) {
    ll_close_receiver(fd);
  }

  close_non_canonical(fd, &oldtio);
  return 0;
}

