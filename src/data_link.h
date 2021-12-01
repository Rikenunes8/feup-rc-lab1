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
 * @brief Send a SET frame and receive a UA frame
 * 
 * @param fd File descriptor of the serial port
 * @return int 0 on success or -1 on failure
 */
int ll_open_transmitter(int fd);

/**
 * @brief Receive SET frame and send UA frame
 * 
 * @param fd File descriptor of the serial port
 * @return int 0 on success or -1 on failure
 */
int ll_open_receiver(int fd);

/**
 * @brief Establish the comunication between transmitter and receiver
 * 
 * @param port Path to the port to open
 * @param status Transmissor/Receiver
 * @return int File descriptor of the port oppened or -1 on failure
 */
int llopen(char* port, int status);

/**
 * @brief Create INFORMATION frame with data received, apply byte stuffing, send the frame by the serial port and wait for a answer from receiver. If the answer is a REJ then resend the information frame, if it is a RR then ends. Resend information frame for a maximum value of times and it has a time out when no answer is received from receiver.
 * 
 * @param fd File descriptor of the serial port
 * @param data Array with the data to be send 
 * @param length Size of the data
 * @return int 0 on sucess or -1 on failure
 */
int llwrite(int fd, uchar* data, int length);

/**
 * @brief 
 * 
 * @param fd File descriptor of the serial port
 * @param buffer Array to be set with the data received after parsing it
 * @return int Size of the data on success or -1 on failure
 */
int llread(int fd, uchar* buffer);

/**
 * @brief Send a DISC frame, receive a DISC frame and send a UA frame
 * 
 * @param fd File descriptor of the serial port
 * @return int 0 on success or -1 on faliure
 */
int ll_close_transmitter(int fd);

/**
 * @brief Receive a DISC frame, send a DISC frame and receive a UA frame
 * 
 * @param fd File descriptor of the serial port
 * @return int 0 on success or -1 on faliure
 */
int ll_close_receiver(int fd);

/**
 * @brief Close the comunication between transmitter and receiver
 * 
 * @param fd File descriptor of the serial port to be closed
 * @param status Transmissor/Receiver
 * @return int 0 on success and -1 on failure
 */
int llclose(int fd, int status);
