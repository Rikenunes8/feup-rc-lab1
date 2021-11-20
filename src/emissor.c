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
#include "dl_protocol.h"
#include "sp_config.h"
#include "alarm.h"


int main(int argc, char** argv) {
  
  if ( (argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) &&
        (strcmp("/dev/ttyS1" , argv[1])!=0) &&
        (strcmp("/dev/ttyS10", argv[1])!=0) &&
        (strcmp("/dev/ttyS11", argv[1])!=0) )) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }
  
  printf("Establishing connection\n");
  int fd = llopen(argv[1], TRANSMITTER);
  
  printf("Ending connection\n");
  int ret = llclose(fd, TRANSMITTER);

  printf("Closing\n");
  return ret;
}
