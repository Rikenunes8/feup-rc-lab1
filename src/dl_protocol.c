#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dl_protocol.h"
#include "sp_config.h"
#include "alarm.h"
#include "state_machine.h"

#define N_CMDS 2
#define N_ANSW 5



char get_BCC_1(char a, char b) {
  return a ^ b;
}

char get_BCC_2(char* data, int length) {
  char bcc2 = data[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ data[i];
  }
  return bcc2;
}

int create_sv_un_frame(char* frame, char control, int who) {
  int is_command = control == SET || control == DISC;
  int is_answer = control == UA || control == RR_0 || control == RR_1 || control == REJ_0 || control == REJ_1;
  
  frame[0] = FLAG;
  if ((is_command && who == TRANSMITTER) || (is_answer && who == RECEIVER))
    frame[1] = A_1;
  else if ((is_command && who == RECEIVER) || (is_answer && who == TRANSMITTER))
    frame[1] = A_2;
  else
    return -1;
  frame[2] = control;
  frame[3] = get_BCC_1(frame[1], frame[2]);
  frame[4] = FLAG;

  return 0;
}

int read_sv_un_frame(int fd, char address, char control) {
  char bcc1 = get_BCC_1(address, control);
  State_machine* sm = create_sm(address, control, bcc1);
  printf("After create machine\n");
  char byte;
  while (sm->state != STOP && !finish && !send_frame) {
    printf("While: %d : %d\n", finish, send_frame);
    switch (sm->state) {
      case START:
        printf("START\n");
        break;
      case FLAG_RCV:
        printf("FLAG_RCV\n");
        break;
      case A_RCV:
        printf("A_RCV\n");
        break;
      case C_RCV:
        printf("C_RCV\n");
        break;
      case BCC_OK:
        printf("BCC_OK\n");
        break;
      case STOP:
        printf("Stop\n");
        break;
      default:
        printf("NADA\n");
    }
    if (read(fd, &byte, sizeof(char)) > 0) {
      printf("byte: %x\n", byte);
      event_handler_sm(sm, byte);
    }
  }
  printf("before destroy mavhchine\n");
  destroy_sm(sm);

  if (sm->state == STOP)
    return SV_UN_SIZE;
  else
    return -1;
}

int write_frame(int fd, char* frame, unsigned size) {
  return write(fd, frame, size);
}




int ll_open_transmitter(int fd) {
  char buffer[MAX_SIZE];
  
  if (create_sv_un_frame(buffer, SET, TRANSMITTER) < 0) {
    return -1;
  }

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, buffer, SV_UN_SIZE);
      printf("SET command sent\n");

      alarm(TIME_OUT);
      send_frame = FALSE;
    }
    printf("Before read_sv_un_frame\n");
    int read_value = read_sv_un_frame(fd, A_1, UA);
    printf("After read_sv_un_frame\n");
    if (read_value >= 0) {
      printf("good read\n");
      alarm(0);
      finish = TRUE; 
    }
  }

  if (read_value < 0) {
    printf("fail\n");
    return -1;
  }
  printf("UA answer received\n");

  return fd;
}

int ll_open_receiver(int fd) {
  printf("Before read_sv_un_frame\n");
  int res = read_sv_un_frame(fd, A_1, SET);
  printf("SET command received\n");

  char buffer[MAX_SIZE];
  create_sv_un_frame(buffer, UA, RECEIVER);

  write_frame(fd, buffer, SV_UN_SIZE);
  printf("UA answer sent\n");

  return 0;
}

int llopen(char* port, int who) {
  printf("Before open_non_canonical\n");

  int fd = open_non_canonical(port, &oldtio, 0, 5);
  if (fd < 0) {
    return -1;
  }
  printf("Before set_alarm\n");
  set_alarm();

  printf("After setALarm\n");
  if (who == TRANSMITTER) {
    int ok = ll_open_transmitter(fd);
    if (ok < 0) {
      close_non_canonical(fd, &oldtio);
      return -1;
    }
    else {
      printf("After ll_open_transmitter\n");
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
      printf("After ll_open_receiver\n");
      return fd;
    }
  }
  printf("After \n");
  return -1;
}

int llwrite(int fd, char* buffer, int length) {
  return 0;
}

int llread(int fd, char* buffer) {
  return 0;
}

int llclose(int fd) {
  close_non_canonical(fd, &oldtio);
  return 0;
}