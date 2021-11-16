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
#include "sp_config.h"

volatile int STOP=FALSE;



int main(int argc, char** argv)
{
  int fd,c, res;
  struct termios oldtio;
  char buf[255];

  if ( (argc < 2) ||
          ((strcmp("/dev/ttyS0", argv[1])!=0) &&
          (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS10", argv[1])!=0) &&
          (strcmp("/dev/ttyS11", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  fd = open_non_canonical(argv[1], &oldtio, 0, 5);


  finish_setting_messages();
  Frame SET_command, UA_answer, frm;
  set_command_SET(&SET_command);
  set_answer_UA(&UA_answer);
  UA_answer.frame[1] = 0x42; // Test error reading


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
  
  close_non_canonical(fd, &oldtio);
  return 0;
}
