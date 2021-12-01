#pragma once

#include "macros.h"


typedef struct {
  char port[20];
  int baudRate;
  uchar sequenceNumber;
  unsigned int timeout;
  unsigned int numTransmissions;
  uchar frame[MAX_STUF_SIZE];
  int frame_size;
} LinkLayer;


/**
 * @brief 
 * 
 * @param fd 
 * @return int 0 on success or -1 on failure
 */
int ll_open_transmitter(int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 0 on success or -1 on failure
 */
int ll_open_receiver(int fd);

/**
 * @brief 
 * 
 * @param port Path to the port to open
 * @param status Transmissor/Receiver
 * @return int File descriptor of the port oppened or -1 on failure
 */
int llopen(char* port, int status);

/**
 * @brief 
 * 
 * @param fd 
 * @param buffer 
 * @param length 
 * @return int 
 */
int llwrite(int fd, uchar* buffer, int length);

/**
 * @brief 
 * 
 * @param fd 
 * @param buffer 
 * @return int 
 */
int llread(int fd, uchar* buffer);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int ll_close_transmitter(int fd);

/**
 * @brief 
 * 
 * @param fd 
 * @return int 
 */
int ll_close_receiver(int fd);

/**
 * @brief 
 * 
 * @param fd File descriptor of the file to be closed
 * @param status Transmissor/Receiver
 * @return int 0 on success and -1 on failure
 */
int llclose(int fd, int status);
