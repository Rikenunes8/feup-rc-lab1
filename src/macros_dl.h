#pragma once

#include "macros.h"

//#define BAUDRATE B38400
#define BAUDRATE B19200

#define INFORMATION 0
#define SUPERVISION 1
#define UNNUMBERED  2

#define TIME_OUT    3
#define MAX_RESENDS 3 

#define ADDRS_BYTE 1
#define CNTRL_BYTE 2
#define BCC1_BYTE  3 
#define DATA_BEGIN 4

#define ESC  0x7D
#define ESC_STUF 0x5D
#define FLAG_STUF 0x5E


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
#define BCC(x,y) x^y

#define SU_SIZE 5
#define MAX_FRAME_SIZE (1 + MAX_SIZE + SU_SIZE)
#define MAX_STUF_FRAME_SIZE (2 * (1 + MAX_SIZE) + SU_SIZE)

// Eficiency tests
#define EFFICIENCY_TEST 1
#define FER       2
#define T_PROP    0
