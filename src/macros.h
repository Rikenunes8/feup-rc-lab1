#pragma once


#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1
#define MAX_SIZE 255
#define TIME_OUT 3
#define MAX_RESENDS 3
#define MSG_SIZE 5


#define FLAG    0x7E
#define A_1     0x03
#define A_2     0x01
#define SET     0x03
#define DISC    0x0B
#define UA      0x07
#define S_0     0x00
#define S_1     0x40
#define RR_0    0x05
#define RR_1    0x85
#define REJ_0   0x01
#define REJ_1   0x81
