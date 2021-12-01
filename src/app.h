#pragma once

#include "macros.h"

typedef struct {
  int fd; // File descriptor of the serial port*/
  int status; // TRANSMITTER | RECEIVER
  char filename[100]; // Path to the file to be transmitted or received
} ApplicationLayer;

int parse_args(char* port, int argc, char** argv);

int buildControlPacket(uchar* packet, uchar type, off_t* size);

int buildDataPacket(uchar* packet, uchar n, uchar* data, int data_size);

int transmitter();

int receiver();