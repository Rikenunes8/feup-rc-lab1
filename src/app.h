#pragma once

#include "macros.h"

typedef struct {
  int fd; // File descriptor of the serial port*/
  int status; // TRANSMITTER | RECEIVER
  char filename[100]; // Path to the file to be transmitted or received
} ApplicationLayer;

/**
 * @brief Parse args
 * 
 * @param port String to be set with the path of the serial port
 * @param argc Number of arguments
 * @param argv Arguments
 * @return int 0 on success or -1 on failure
 */
int parse_args(char* port, int argc, char** argv);

/**
 * @brief Create a control packet according to the args received
 * 
 * @param packet Array to be set with the packet's bytes
 * @param type START/END (0x02/0x03) byte, meaning the start or the end of data transmisson, respectivelly
 * @param size Size of the file to be transmitted in bytes
 * @return int Size of the packet
 */
int buildControlPacket(uchar* packet, uchar type, off_t size);

/**
 * @brief Create a data packet
 * 
 * @param packet Array to be set with the packet's bytes
 * @param n Sequence number of the packet
 * @param data Array with the data to be transmitted
 * @param data_size Size of the data
 * @return int Size of the packet
 */
int buildDataPacket(uchar* packet, uchar n, uchar* data, int data_size);

/**
 * @brief Read the file to be transmitted and send its content. Open the file to be transmitted, send a start control packet followed by several data packets until all the file be read and then send an end control packet
 * 
 * @return int 0 on success or -1 on faliure
 */
int transmitter();

/**
 * @brief Receive data and write it on a new file. Read until an end control packet is received. When a start control packet is received, a file is open to be written with the data received on data packets
 * 
 * @return int 0 on success or -1 on faliure
 */
int receiver();