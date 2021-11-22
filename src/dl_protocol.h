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





void create_su_frame(uchar* frame, uchar address, uchar control);
int create_info_frame(uchar* frame, uchar control, uchar* data, int data_length);
int read_su_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame);
int read_info_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame);
int write_frame(int fd, uchar* frame, unsigned size);


int ll_open_transmitter(int fd);

int ll_open_receiver(int fd);

int llopen(char* port, int status);

int llwrite(int fd, uchar* buffer, int length);

int llread(int fd, uchar* buffer);

int llclose(int fd, int status);
