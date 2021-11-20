#pragma once

#include "macros.h"



typedef struct {
  char port[20];
  int baudRate;
  unsigned int sequenceNumber;
  unsigned int timeout;
  unsigned int numTransmissions;
  char frame[MAX_SIZE];
} LinkLayer;



struct termios oldtio;
unsigned sequence_number;

int create_sv_un_frame(uchar* frame, uchar control, int who);
int create_info_frame(uchar* frame, uchar control, uchar* data, int data_length);
int read_sv_un_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame);
int read_info_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame);
int write_frame(int fd, uchar* frame, unsigned size);


int ll_open_transmitter(int fd);

int ll_open_receiver(int fd);

int llopen(char* port, int who);

int llwrite(int fd, uchar* buffer, int length);

int llread(int fd, uchar* buffer);

int llclose(int fd, int who);
