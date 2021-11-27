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
 * @brief Create Supervision/Unnumbered frame given the addess byte and control byte.
 * BCC_1 is calculated with address and control bytes and FLAGs are added to both edges of the frame
 * 
 * @param frame Array of bytes to be set
 * @param address Address byte to set frame with
 * @param control Control byte to set frame with
 */
void create_su_frame(uchar* frame, uchar address, uchar control);
/**
 * @brief Create Information frame given the control byte and data.
 * Address is always the same, BCC_1 is calculated with address and control bytes, BCC_2 is calculated based on data bytes and FLAGs are added to both edges of the frame
 * 
 * @param frame Array of bytes to be set
 * @param control Control byte to set frame with
 * @param data Data bytes to set frame with
 * @param data_length Size of data
 * @return int Size of frame
 */
int create_info_frame(uchar* frame, uchar control, uchar* data, int data_length);
/**
 * @brief Read Supervision/Unnumbered frame, byte by byte
 * 
 * @param fd File descriptor of the file to be read
 * @param address Address byte that can be acceptd
 * @param controls Control bytes that can be accepted
 * @param n_controls Number of control bytes that can be accepted
 * @param frame Array to be set with the information read
 * @return int Index of control chosen or -1 on failure
 */
int read_su_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame);
/**
 * @brief Read Information frame, byte by byte
 * 
 * @param fd File descriptor of the file to be read
 * @param address Address byte that can be acceptd
 * @param controls Control bytes that can be accepted
 * @param n_controls Number of control bytes that can be accepted
 * @param frame Array to be set with the information read
 * @return int Size of frame
 */
int read_info_frame(int fd, uchar address, uchar* controls, int n_controls, uchar* frame);
/**
 * @brief Write size bytes from frame on the with with the corresponding file descriptor fd.
 * 
 * @param fd File descriptor of the file to be write
 * @param frame Array with the information to be write
 * @param size Size of frame
 * @return int Number of bytes written or -1 on failure
 */
int write_frame(int fd, uchar* frame, unsigned size);

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
