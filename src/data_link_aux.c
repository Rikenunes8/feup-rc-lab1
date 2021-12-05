#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "data_link_aux.h"
#include "state_machine.h"
#include "log.h"
#include "macros_dl.h"

#include <time.h> // EFFICIENCY TEST
#include <stdlib.h> // EFFICIENCY TEST


extern int finish;
extern int send_frame;

uchar BCC_2(uchar* data, int length) {
  uchar bcc2 = data[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ data[i];
  }
  return bcc2;
}

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
  if (EFFICIENCY_TEST) usleep(T_PROP);
  return write(fd, frame, size);
}


int byte_stuffing(uchar* frame, int length) {
  int final_len = DATA_BEGIN;

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == FLAG) {
      frame[final_len++] = ESC;
      frame[final_len++] = FLAG_STUF;
    }
    else if (aux[i] == ESC) {
      frame[final_len++] = ESC;
      frame[final_len++] = ESC_STUF;
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
    if (aux[i] == ESC) {
      if (aux[i+1] == FLAG_STUF) {
        frame[final_len++] = FLAG;
        i++;
      }
      else if(aux[i+1] == ESC_STUF) {
        frame[final_len++] = ESC;
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


int open_non_canonical(char* file, struct termios *oldtio, int baudrate, int vtime, int vmin) {
  struct termios newtio;

  int fd = open(file, O_RDWR | O_NOCTTY );
  if (fd < 0) {
    log_err("open() - openning port");
    return -1; 
  }

  if ( tcgetattr(fd, oldtio) == -1) { /* save current port settings */
    log_err("tcgetattr() - getting old configuration");
    return -1;
  }

  
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = vtime; /* inter-character timer unused */
  newtio.c_cc[VMIN] = vmin; /* blocking read until vmin chars received */

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    log_err("tcsetattr() - setting new configuration");
    return -1;
  }
  log_msg("New termios structure set");

  return fd;
}

int close_non_canonical(int fd, struct termios* oldtio) {
  sleep(1);
  
  if (tcsetattr(fd,TCSANOW, oldtio) == -1) {
    log_err("tcsetattr() - setting old configuration");
    return -1;
  }

  log_msg("Old termios structure set");
  if (close(fd) < 0) {
    log_err("close() - closing port");
    return -1;
  }

  return 0;
}

// ----------------- EFFICIENCY TEST -----------------------

uchar generate_error_BCC(uchar byte, int bcc) {
  int prob = (rand()%100) + 1;
  if (prob <= FER) {
    log_bcc_error(bcc);
    byte = rand()%256;
  }
  return byte;
}

void start_time(struct timespec *start_time){
  clock_gettime(CLOCK_MONOTONIC, start_time);
}

double ellapsed_time_ms(struct timespec start_time) {
  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  double elapsed = (current_time.tv_sec-start_time.tv_sec)*1000+((current_time.tv_nsec-start_time.tv_nsec)/10e6);
  return elapsed;
}

// ---------------------------------------------------------