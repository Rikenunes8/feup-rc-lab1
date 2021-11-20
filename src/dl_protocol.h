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


// -------- AUXILIAR FUNCTIONS -----------
char get_BCC_1(char a, char b);

char get_BCC_2(char* data, int length);

// --------------------------------------------



int llopen(char* port, int who);

int llwrite(int fd, char* buffer, int length);

int llread(int fd, char* buffer);

int llclose(int fd, int who);
