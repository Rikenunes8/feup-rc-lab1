#pragma once

#include "macros.h"



/**
 * @brief XOR between all bytes in data
 * 
 * @param data Array with bytes
 * @param length Size of data
 * @return uchar XOR
 */
uchar BCC_2(uchar* data, int length);

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
 * @param fd File descriptor of the serial port
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
 * @param fd File descriptor of the serial port
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
 * @param fd File descriptor of the serial port
 * @param frame Array with the information to be write
 * @param size Size of frame
 * @return int Number of bytes written or -1 on failure
 */
int write_frame(int fd, uchar* frame, unsigned size);

/**
 * @brief Given a frame, all the data fields are stuffed. It means 0x7D5E replace all 0x7E bytes and 0x7D5D replace all 0x7D bytes on data fields (all bytes on frame but the first 4 and the last one). BCC_2 is count as a data field.
 * 
 * @param frame Frame with current content and that will be stuffed
 * @param length Size of the frame given
 * @return int Size of the new frame
 */
int byte_stuffing(uchar* frame, int length);

/**
 * @brief Given a frame, all the data fields are destuffed. It means 0x7E replace all 0x7D5E bytes and 0x7D replace all 0x7D5D bytes on data fields (all bytes on frame but the first 4 and the last one). BCC_2 is count as a data field.
 * 
 * @param frame Frame with current content and that will be stuffed
 * @param length Size of the frame given
 * @return int Size of the new frame
 */
int byte_destuffing(uchar* frame, int length);

/**
 * @brief Open the serial port
 * 
 * @param file Port name
 * @param oldtio Structure to keep the initial settings
 * @param vtime VTIME
 * @param vmin VMIN
 * @return int File descriptor on success or -1 on failure
 */
int open_non_canonical(char* file, struct termios* oldtio, int baudrate, int vtime, int vmin);
/**
 * @brief Close the serial port
 * 
 * @param fd File descriptor of the port
 * @param oldtio Structure with initial settings to be set
 * @return int 0 on success or -1 on failure
 */
int close_non_canonical(int fd, struct termios* oldtio);
