
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


int buildControlPacket(uchar* packet, uchar type, off_t* size) {
  printf("%x  ----  %d", *size, *size);
  uchar offsize = (uchar)sizeof(off_t);
  packet[0] = type;
  packet[1] = FILE_SIZE;
  packet[2] = offsize;
  memcpy(&packet[3], size, offsize);
  uchar filename_len = (uchar)strlen(app_layer.filename);
  packet[3+offsize] = FILE_NAME;
  packet[4+offsize] = filename_len;
  memcpy(&packet[5+offsize], &app_layer.filename, filename_len);

  return 5 + offsize + filename_len;
}

int transmitter() {
  int fd = open(app_layer.filename, O_RDONLY);

  struct stat file_info;
  fstat(fd, &file_info);

  uchar buffer[MAX_SIZE];
  int size = buildControlPacket(buffer, PACK_START, &file_info.st_size);
  if (llwrite(app_layer.fd, buffer, size) < 0) {
    return -1;
  }


  while (FALSE) {

  }


  buffer[0] = PACK_END;
  if (llwrite(app_layer.fd, buffer, size) < 0) {
    return -1;
  }
  return 0;
}

int receiver() {
  uchar buffer[MAX_SIZE];

  while (TRUE) {
    int size = llread(app_layer.fd, buffer);

    if (buffer[0] == PACK_START) {
      off_t filesize = 0;
      int next_tlv = 1;
      
      while (next_tlv != size) {
        if (buffer[next_tlv] == FILE_SIZE) {
          for (int i = next_tlv+2; i < next_tlv+2+buffer[next_tlv+1]; i++) {
            filesize += buffer[i] << (8*(i-(next_tlv+2)));
          }
          next_tlv += buffer[next_tlv+1]+2;
        }
        else if (buffer[next_tlv] == FILE_NAME) {
          int filename_len = buffer[next_tlv+1];
          memcpy(app_layer.filename, &buffer[next_tlv+2], filename_len);
          app_layer.filename[filename_len] = '\0';
          next_tlv += filename_len + 2;
        }
      }
    }
    else if (buffer[0] == PACK_END) {
      break;
    }
    else if (buffer[0] == PACK_DATA) {

    }
    else {
      printf("Fail");
    }
  }
  return 0;
}


int main(int argc, char** argv) {
  char port[12];
  if (parse_args(port, argc, argv) < 0) {
    printf("Usage:\tapp status port\nstatus = {transmitter, receiver}\nport = {0, 10, 11}\n");
    return -1;
  }
  strcpy(app_layer.filename, "pinguim.gif");


  log_msg("Establishing connection");
  app_layer.fd = llopen(port, app_layer.status);


  if (app_layer.status == TRANSMITTER) {
    log_msg("Transfering data");
    /*uchar buffer[] = {0x02, 0x03, 0x04, 0x7e, 0x05};
    llwrite(app_layer.fd, buffer, 5);*/
    transmitter();
  }
  else {
    log_msg("Receiving data");
    /*uchar buffer[MAX_SIZE];
    int size = llread(app_layer.fd, buffer);
    for (int i = 0; i < size; i++) {
      printf(":%x", buffer[i]);
    }
    printf("\n");*/
    receiver();
  }
  


  log_msg("Ending connection");
  int ret = llclose(app_layer.fd, app_layer.status);

  log_msg("Closing");
  return ret;
}