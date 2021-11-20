#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dl_protocol.h"
#include "sp_config.h"
#include "alarm.h"
#include "state_machine.h"


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

/*
int byteStuffing(char* frame, int length) {
  int fullLen = length + 6; int finalLen = DATA_BEGIN;

  char aux[fullLen];

  for (int i = DATA_BEGIN; i < fullLen; i++) {
    aux[i] = frame[i];
  }

  for (int i = DATA_BEGIN; i < fullLen-1; i++) {
    if (aux[i] == FLAG) {
      frame[finalLen] = ESCAPE;
      frame[finalLen+1] = FLAG_STUFFING;
      finalLen = finalLen + 2;
    }

    else if (aux [i] == ESCAPE) {
      frame[finalLen] = ESCAPE;
      frame[finalLen+1] = ESCAPE_STUFFING;
      finalLen = finalLen + 2;
    }

    else {
      finalLen++;
    }
  }

  return finalLen;
}*/


/*
int byteDestuffing(char* frame, int length) {
  char aux[length + 5]; //bcc2 is included in stuffing
  int finalLen = DATA_BEGIN; int fullLen = length + 5;

  for (int i = DATA_BEGIN; i < fullLen; i++) {
    aux[i] = frame[i];
  }


  for (int i = DATA_BEGIN; i < fullLen; i++, finalLen++) {
    if (aux[i+1] == FLAG_STUFFING) {
      frame[finalLen] = FLAG;
      i++;
    }

    else if(aux[i+1] == ESCAPE_STUFFING) {
      frame[finalLen] = ESCAPE;
      i++;
    }

    else {
      frame[finalLen] = aux[i];
    }

  }

  return finalLen;
} */

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
  char byte;
  while (sm->state != STOP && !finish && !send_frame) {
    if (read(fd, &byte, sizeof(char)) > 0) {
      event_handler_sm(sm, byte);
    }
  }
  destroy_sm(sm);

  if (finish || send_frame)
    return -1;
  else
    return SV_UN_SIZE;
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
      printf("SET frame sent\n");

      alarm(TIME_OUT);
      send_frame = FALSE;
    }
    read_value = read_sv_un_frame(fd, A_1, UA);
    
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

int ll_close_transmitter(int fd) {
  char buffer[MAX_SIZE];
  
  if (create_sv_un_frame(buffer, DISC, TRANSMITTER) < 0) {
    return -1;
  }

  finish = FALSE;
  send_frame = TRUE;
  n_sends = 0;

  int read_value;
  while (!finish) {
    if (send_frame) {
      write_frame(fd, buffer, SV_UN_SIZE);
      printf("DISC frame sent\n");

      alarm(TIME_OUT);
      send_frame = FALSE;
    }
    read_value = read_sv_un_frame(fd, A_1, DISC);


    
    if (read_value >= 0) {
      create_sv_un_frame(buffer, UA, TRANSMITTER); //check if this is ok pls 
      write_frame(fd , buffer, SV_UN_SIZE);
      alarm(0);
      finish = TRUE; 
    }
    else if (n_sends >= MAX_RESENDS) {
      printf("Limit of resends\n");
      finish = TRUE;
    }
  }
}

int ll_open_receiver(int fd) {
  int res = read_sv_un_frame(fd, A_1, SET);
  printf("SET frame received\n");


  char buffer[MAX_SIZE];
  create_sv_un_frame(buffer, UA, RECEIVER);
  // create_sv_un_frame(buffer, SET, RECEIVER); // Test failed answer

  write_frame(fd, buffer, SV_UN_SIZE);
  printf("UA frame sent\n");

  return 0;
}

int ll_close_receiver(int fd) {
  int res = read_sv_un_frame(fd, A_1, DISC);
  printf("DISC frame received\n");

  char buffer[MAX_SIZE];
  create_sv_un_frame(buffer, DISC, RECEIVER);

  write_frame(fd, buffer, SV_UN_SIZE);  //sent DISC

  res = read_sv_un_frame(fd, A_1, UA);
  printf("UA frame received\n");

  return 0;
}

int llopen(char* port, int who) {

  int fd = open_non_canonical(port, &oldtio, 0, 5);
  if (fd < 0) {
    return -1;
  }
  set_alarm();

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

int llwrite(int fd, char* buffer, int length) {
  return 0;
}

int llread(int fd, char* buffer) {
  return 0;
}

int llclose(int fd) {
  if (fd < 0) {
    return -1;
  }
  close_non_canonical(fd, &oldtio);
  return 0;
}
