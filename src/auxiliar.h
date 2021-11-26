#pragma once

#include "macros.h"

// -------- AUXILIAR FUNCTIONS -----------

/**
 * @brief XOR between 2 bytes
 * 
 * @param a First byte
 * @param b Second byte
 * @return uchar a^b
 */
uchar get_BCC_1(uchar a, uchar b);

/**
 * @brief XOR between all bytes in data
 * 
 * @param data Array with bytes
 * @param length Size of data
 * @return uchar XOR
 */
uchar get_BCC_2(uchar* data, int length);

// --------------------------------------------

/**
 * @brief Given a frame, all the data fields are stuffed. It means 0x7D5E replace all 0x7E bytes and 0x7D5D replace all 0x7D bytes on data fields (all bytes on frame but the first 4 and the last one). BCC_2 is count as a data field.
 * 
 * @param frame Frame with current content and that will be stuffed
 * @param length Size of the frame given
 * @return int Size of the new frame
 */
int byteStuffing(uchar* frame, int length);

/**
 * @brief Given a frame, all the data fields are destuffed. It means 0x7E replace all 0x7D5E bytes and 0x7D replace all 0x7D5D bytes on data fields (all bytes on frame but the first 4 and the last one). BCC_2 is count as a data field.
 * 
 * @param frame Frame with current content and that will be stuffed
 * @param length Size of the frame given
 * @return int Size of the new frame
 */
int byteDestuffing(uchar* frame, int length);

void print_frame(uchar* frame, int length);