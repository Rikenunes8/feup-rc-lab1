#include <termios.h>
#include <unistd.h>
#include <string.h>
#include "data_link_aux.h"
#include "state_machine.h"

extern int finish;
extern int send_frame;

void create_su_frame(uchar* frame, uchar address, uchar control) {
  frame[0] = FLAG;
  frame[1] = address;
  frame[2] = control;
  frame[3] = BCC(frame[1], frame[2]);
  frame[4] = FLAG;
}

int create_info_frame(uchar* frame, uchar control, uchar* data, int data_length) {
  frame[0] = FLAG;
  frame[1] = A_1;
  frame[2] = control;
  frame[3] = BCC(frame[1], frame[2]);
  for (int i = 0; i < data_length; i++) {
    frame[4+i] = data[i];
  }
  frame[4+data_length] = BCC_2(data, data_length);
  frame[4+data_length+1] = FLAG;

  return 4+data_length+2;
}

int read_su_frame(int fd, uchar address, uchar* wanted_controls, int n_controls, uchar* frame) {
  State_machine* sm = create_sm(address, wanted_controls, n_controls);
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

int read_info_frame(int fd, uchar address, uchar* wanted_controls, int n_controls, uchar* frame) {
  State_machine* sm = create_sm(address, wanted_controls, n_controls);
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

uchar BCC_2(uchar* data, int length) {
  uchar bcc2 = data[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ data[i];
  }
  return bcc2;
}

int byte_stuffing(uchar* frame, int length) {
  int final_len = DATA_BEGIN;

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == FLAG) {
      frame[final_len++] = ESCAPE;
      frame[final_len++] = FLAG_STUFFING;
    }
    else if (aux[i] == ESCAPE) {
      frame[final_len++] = ESCAPE;
      frame[final_len++] = ESCAPE_STUFFING;
    }
    else {
      frame[final_len++] = aux[i];
    }
  }
  frame[final_len++] = FLAG;
  return final_len;
}

int byte_destuffing(uchar* frame, int length) {
  int final_len = DATA_BEGIN; 

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == ESCAPE) {
      if (aux[i+1] == FLAG_STUFFING) {
        frame[final_len++] = FLAG;
        i++;
      }
      else if(aux[i+1] == ESCAPE_STUFFING) {
        frame[final_len++] = ESCAPE;
        i++;
      }
    }
    else {
      frame[final_len++] = aux[i];
    }
  }
  frame[final_len++] = FLAG;
  return final_len;
} 
