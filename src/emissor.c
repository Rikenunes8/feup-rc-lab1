/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "protocol.h"
#include "alarm.h"

int count = 0, allgood = FALSE, send = TRUE;
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
  //(void) signal(SIGALRM, alarm_handler);  
  set_alarm(); // instala  rotina que atende interrupcao

  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255];
  int i, sum = 0, speed = 0;

  

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

  // Frames checkers
  print_frame(&SET_command, "SET command");
  print_frame(&UA_answer, "UA answer");
  printf("\n\n");
  // ---------------------

  while (!allgood && count < MAX_RESENDS) {
    if (!send) {
      continue;
    }
    res = write(fd, SET_command.frame, SET_command.size);
    send = FALSE;     // Prepare to not send the frame again until the time out
    alarm(TIME_OUT);  // Set alarm to TIME_OUT seconds

    int error_reading = FALSE;
    char rbuf[1];
    frm.size = 0;
    int parse = FALSE;

    while (!STOP) { 
      if (read(fd, rbuf, 1) == -1) {
        printf("Error reading\n");
        error_reading = TRUE;
        break;
      }
      
      if (rbuf[0] == FLAG) {
        parse = !parse;
        buf[frm.size++] = rbuf[0];
        if (!parse) {
          STOP = TRUE;
        }
      }
      else if (parse) {
        buf[frm.size++] = rbuf[0];
      }
      
    }
    STOP = FALSE;
    if (error_reading) {
      continue;
    }

    set_frame_from_buffer(buf, &frm);
    print_frame(&frm, "Frame received");

    if (frame_cmp(&UA_answer, &frm)) {
      allgood = TRUE;
      printf("UA answer received\n");
    } 
    else {
      printf("Wrong answer\n");
    }
  }


  sleep(1);
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(1);
  }

  close(fd);
  return 0;
}
