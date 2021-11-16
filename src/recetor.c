/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocol.h"

volatile int STOP=FALSE;

void set_new_termios(struct termios *newtio) {
  bzero(newtio, sizeof(*newtio));
  newtio->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio->c_iflag = IGNPAR;
  newtio->c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio->c_lflag = 0;

  newtio->c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio->c_cc[VMIN] = 5; /* blocking read until 5 chars received */

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) próximo(s) caracter(es)
  */
}

int main(int argc, char** argv)
{
  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];

  if ( (argc < 2) ||
          ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS11", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }


  /*
  Open serial port device for reading and writing and not as controlling tty
  because we don't want to get killed if linenoise sends CTRL-C.
  */


  fd = open(argv[1], O_RDWR | O_NOCTTY );
  if (fd <0) {perror(argv[1]); exit(-1); }

  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(1);
  }

  set_new_termios(&newtio);

  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(1);
  }
  printf("New termios structure set\n");

  finish_setting_messages();

  Frame SET_command, UA_answer, frm;
  set_command_SET(&SET_command);
  set_answer_UA(&UA_answer);
  UA_answer.frame[1] = 0x42;


  char rbuf[1];
  frm.size = 0;
  int parse = FALSE;
  while (STOP==FALSE) { /* loop for input */
    res = read(fd, rbuf, 1);

    if (rbuf[0] == FLAG) {
      parse = !parse;
      buf[frm.size++] = rbuf[0];
      if (parse == FALSE) {
        STOP=TRUE;
      }
    }
    else if (parse) {
      buf[frm.size++] = rbuf[0];
    }
  }
  set_frame_from_buffer(buf, &frm);

  print_frame(&frm, "Frame received");

  if (frame_cmp(&SET_command, &frm)) {
    printf("SET command received\n");
    res = write(fd, UA_answer.frame, UA_answer.size);
  }
  


  sleep(1);
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}
