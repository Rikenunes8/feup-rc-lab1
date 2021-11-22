
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "app.h"
#include "dl_protocol.h"
#include "log.h"

static Application_layer app_layer;

int parse_args(char* port, int argc, char** argv) {
  if  (argc != 3) {
    log_err("Wrong number of arguments");
    return -1;
  }
  if (strcmp(argv[1], "transmitter") == 0) {
    app_layer.status = TRANSMITTER;
  } 
  else if (strcmp(argv[1], "receiver") == 0) {
    app_layer.status = RECEIVER;
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
  return 0;
}

int main(int argc, char** argv) {
  char port[12];
  if (parse_args(port, argc, argv) < 0) {
    printf("Usage:\tapp status port\nstatus = {transmitter, receiver}\nport = {0, 10, 11}\n");
    return -1;
  }
  

  log_msg("Establishing connection");
  app_layer.fd = llopen(port, app_layer.status);



  if (app_layer.status == TRANSMITTER) {
    log_msg("Transfering data");
    uchar buffer[] = {0x02, 0x03, 0x04, 0x05};
    llwrite(app_layer.fd, buffer, 4);
  }
  else {
    log_msg("Receiving data");
    uchar buffer[MAX_SIZE];
    int size = llread(app_layer.fd, buffer);
    for (int i = 0; i < size; i++) {
      printf(":%x", buffer[i]);
    }
    printf("\n");
  }
  


  log_msg("Ending connection");
  int ret = llclose(app_layer.fd, app_layer.status);

  log_msg("Closing");
  return ret;
}