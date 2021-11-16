#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "macros.h"

int open_non_canonical(char* file, struct termios *oldtio, int vtime, int vmin) {
  struct termios newtio;

  int fd = open(file, O_RDWR | O_NOCTTY );
  if (fd <0) {perror(file); exit(-1); }

  if ( tcgetattr(fd, oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(1);
  }

  
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = vtime; /* inter-character timer unused */
  newtio.c_cc[VMIN] = vmin; /* blocking read until 5 chars received */

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) próximo(s) caracter(es)
  */
  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(1);
  }
  printf("New termios structure set\n");

  return fd;
}

int close_non_canonical(int fd, struct termios* oldtio) {
  sleep(1);
  
  if (tcsetattr(fd,TCSANOW, oldtio) == -1) {
    perror("tcsetattr");
    exit(1);
  }

  close(fd);
  return 0;
}