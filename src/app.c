
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dl_protocol.h"
#include "log.h"




int main(int argc, char** argv) {
  int who;
  char port[12];
  if  (argc != 3) {
    log_err("Wrong number of arguments");
    return -1;
  }
  if (strcmp(argv[1], "transmitter") == 0) {
    who = TRANSMITTER;
  } 
  else if (strcmp(argv[1], "receiver") == 0) {
    who = RECEIVER;
  }
  else {
    log_err("Second argument is wrong");
    return -1;
  }

  int nport = atoi(argv[2]);
  if (nport != 0 && nport != 10 && nport != 11) {
    log_err("Third argument is not a valid port");
    return -1;
  }
  snprintf(port, 12, "/dev/ttyS%d", nport);


  printf("Establishing connection\n");
  int fd = llopen(port, who);

  if (who == TRANSMITTER) {
    printf("Transfering data\n");
    uchar buffer[] = {0x02, 0x03, 0x04, 0x05};
    llwrite(fd, buffer, 4);
  }
  else {
    printf("Receiving data\n");
    uchar buffer[MAX_SIZE];
    int size = llread(fd, buffer);
    for (int i = 0; i < size; i++) {
      printf(":%x", buffer[i]);
    }
    printf("\n");
  }
  
  printf("Ending connection\n");
  int ret = llclose(fd, who);

  printf("Closing\n");
  return ret;
}