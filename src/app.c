
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

  int nport = atoi(argv[2]); //atoi returns 0 even if its not a digit: if input was "//" nport would be 0 incorrectly
  if (strcmp(argv[2], "0") != 0 && strcmp(argv[2], "10") != 0 && strcmp(argv[2], "11") != 0) {
    log_err("Third argument is not a valid port");
    return -1;
  }
  snprintf(port, 12, "/dev/ttyS%d", nport);
  return 0;
}

int buildControlPacket(uchar* packet, uchar type, off_t* size) {
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

int buildDataPacket(uchar* packet, uchar n, uchar* data, uchar data_size) {
  packet[0] = PACK_DATA;
  packet[1] = n % 256;
  packet[2] = data_size / 256;
  packet[3] = data_size % 256;
  memcpy(&packet[4], data, data_size);

  return data_size+4;
}

int transmitter() {
  int size;
  int fd = open(app_layer.filename, O_RDONLY);
  if (fd < 0) {
    log_err("Could not open file to be transmitted");
    return -1;
  }

  struct stat file_info;
  fstat(fd, &file_info);

  uchar packet[MAX_PACK_SIZE];
  size = buildControlPacket(packet, PACK_START, &file_info.st_size);
  if (llwrite(app_layer.fd, packet, size) < 0) {
    return -1;
  }

  uchar sequence_number = 0;
  uchar data[MAX_DATA_SIZE];
  int data_size;
  do {
    data_size = read(fd, data, MAX_DATA_SIZE);

    size = buildDataPacket(packet, sequence_number, data, data_size);
    if (llwrite(app_layer.fd, packet, size) < 0) {
      return -1; 
    }
    sequence_number++;

  } while (data_size == MAX_DATA_SIZE);
  
  if (close(fd) < 0) {
    log_err("Fail closing file was transmitted");
    return -1;
  }
  

  size = buildControlPacket(packet, PACK_END, &file_info.st_size);
  //packet[0] = PACK_END;
  if (llwrite(app_layer.fd, packet, size) < 0) {
    return -1;
  }
  return 0;
}

int receiver() {
  uchar packet[MAX_PACK_SIZE];
  int fd = open(app_layer.filename, O_WRONLY | O_CREAT);
  if (fd < 0) {
    log_err("Could not open file to be transmitted");
    return -1;
  }


  while (TRUE) {
    int size = llread(app_layer.fd, packet);

    if (packet[0] == PACK_START) {
      off_t filesize = 0;
      int next_tlv = 1;
      
      while (next_tlv != size) {
        if (packet[next_tlv] == FILE_SIZE) {
          for (int i = next_tlv+2; i < next_tlv+2+packet[next_tlv+1]; i++) {
            filesize += packet[i] << (8*(i-(next_tlv+2)));
          }
          next_tlv += packet[next_tlv+1]+2;
        }
        else if (packet[next_tlv] == FILE_NAME) {
          int filename_len = packet[next_tlv+1];
          memcpy(app_layer.filename, &packet[next_tlv+2], filename_len);
          app_layer.filename[filename_len] = '\0';
          strcpy(app_layer.filename, "test.gif"); // TEST
          next_tlv += filename_len + 2;
        }
      }
    }
    else if (packet[0] == PACK_END) {
      break;
    }
    else if (packet[0] == PACK_DATA) {
      int data_size = packet[2]*256 + packet[3];
      if (write(fd, &packet[4], data_size) < 0) {
        log_err("Failed to writing data bytes to the file");
        return -1;
      }
      
    }
    else {
      printf("Fail");
    }
  }

  if (close(fd) < 0) {
    log_err("Fail closing file wrote");
    return -1;
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
