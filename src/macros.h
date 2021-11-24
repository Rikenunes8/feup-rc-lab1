#pragma once


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0
#define RECEIVER    1

#define INFORMATION 0
#define SUPERVISION 1
#define UNNUMBERED  2

#define SU_SIZE 5
#define MAX_DATA_SIZE 256
#define MAX_PACK_SIZE (4 + MAX_DATA_SIZE)
#define MAX_STUF_SIZE (2 * MAX_PACK_SIZE)
#define MAX_STUF_FRAME_SIZE (1 + MAX_STUF_SIZE + SU_SIZE)

#define TIME_OUT    3
#define MAX_RESENDS 3 

#define ADDRS_BYTE 1
#define CNTRL_BYTE 2
#define BCC1_BYTE  3 
#define DATA_BEGIN 4

#define ESCAPE  0x7D
#define FLAG_STUFFING 0x5E
#define ESCAPE_STUFFING 0x5D


#define FLAG    0x7E
#define A_1     0x03
#define A_2     0x01
#define SET     0x03
#define DISC    0x0B
#define UA      0x07
#define RR_0    0x05
#define RR_1    0x85
#define REJ_0   0x01
#define REJ_1   0x81
#define S_0     0x00
#define S_1     0x40

#define PACK_DATA  0x01
#define PACK_START 0x02
#define PACK_END   0x03

#define FILE_SIZE 0x00
#define FILE_NAME 0x01

typedef unsigned char uchar;