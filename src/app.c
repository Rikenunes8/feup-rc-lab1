
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "app.h"
#include "data_link.h"
#include "log.h"
#include "efficiency.h" // EFFICIENCY TEST


#define PACK_DATA  0x01
#define PACK_START 0x02
#define PACK_END   0x03

#define FILE_SIZE 0x00
#define FILE_NAME 0x01
#define MAX_DATA_SIZE (-4 + MAX_SIZE)


static ApplicationLayer al;


int parse_args(char* port, int argc, char** argv) {
  if (argc < 3 || argc > 4) {
    log_err("Wrong number of arguments");
    return -1;
  }


  if (strcmp(argv[1], "transmitter") == 0) {
    al.status = TRANSMITTER;
  } 
  else if (strcmp(argv[1], "receiver") == 0) {
    al.status = RECEIVER;
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


  if (argc == 4) 
    strcpy(al.filename, argv[3]);
  else if (al.status == RECEIVER) {
    strcpy(al.filename, "./");
  }
  else {
    log_err("Wrong number of arguments");
    return -1;
  }

  return 0;
}

int build_control_packet(uchar* packet, uchar type) {
  uchar offsize = (uchar)sizeof(off_t);
  packet[0] = type;
  packet[1] = FILE_SIZE;
  packet[2] = offsize;
  memcpy(&packet[3], &al.filesize, offsize);
  uchar filename_len = (uchar)strlen(al.filename);
  packet[3+offsize] = FILE_NAME;
  packet[4+offsize] = filename_len;
  memcpy(&packet[5+offsize], &al.filename, filename_len);

  return 5 + offsize + filename_len;
}

int build_data_packet(uchar* packet, uchar n, uchar* data, int data_size) {
  packet[0] = PACK_DATA;
  packet[1] = n % 256;
  packet[2] = data_size / 256;
  packet[3] = data_size % 256;
  memcpy(&packet[4], data, data_size);

  return data_size+4;
}

int transmitter() {
  int fd = open(al.filename, O_RDONLY);
  if (fd < 0) {
    log_err("Fail openning file to be transmitted");
    return -1;
  }


  struct stat file_info;
  fstat(fd, &file_info);
  al.filesize = file_info.st_size;

  int size;
  uchar packet[MAX_SIZE];

  size = build_control_packet(packet, PACK_START);
  if (llwrite(al.fd, packet, size) < 0) {
    return -1;
  }
  
  if (EFFICIENCY_TEST) srand(time(NULL));
  if (EFFICIENCY_TEST) start_time();

  uchar sequence_number = 0;
  uchar data[MAX_DATA_SIZE];
  int data_size;
  off_t all_data_read = 0;
  do {
    data_size = read(fd, data, MAX_DATA_SIZE);
    size = build_data_packet(packet, sequence_number, data, data_size);

    if (llwrite(al.fd, packet, size) < 0) {
      break;
    }

    all_data_read += data_size;
    log_progression(all_data_read, al.filesize, TRANSMITTER);

    sequence_number++;
  } while (data_size == MAX_DATA_SIZE && all_data_read != al.filesize);
  
  if (close(fd) < 0) {
    log_err("Fail closing file was transmitted");
  }

  if (EFFICIENCY_TEST) efficiency(al.filesize);
  
  size = build_control_packet(packet, PACK_END);
  if (llwrite(al.fd, packet, size) < 0) {
    return -1;
  }

  return 0;
}

int receiver() {
  uchar packet[MAX_SIZE];
  int fd = -1;
  uchar sequence_number = 0;
  al.filesize = 0;
  off_t new_filesize = 0;
  int transmitting_data = FALSE;

  while (TRUE) {
    int size = llread(al.fd, packet);
    

    if (packet[0] == PACK_START) {
      transmitting_data = TRUE;
      int next_tlv = 1;

      while (next_tlv != size) {
        int index_l = next_tlv+1;
        int index_v = next_tlv+2;
        
        if (packet[next_tlv] == FILE_SIZE) {
          for (int i = 0; i < packet[index_l]; i++) {
            al.filesize += packet[index_v + i] << (8*i);
          }
          next_tlv += packet[index_l]+2;
        }
        else if (packet[next_tlv] == FILE_NAME) {
          int filename_len = packet[index_l];
          char file_name[filename_len];
          memcpy(file_name, &packet[index_v], filename_len);
          file_name[filename_len] = '\0';
          strcat(al.filename, file_name);
          
          fd = open(al.filename, O_WRONLY | O_CREAT, 0777);
          if (fd < 0) {
            log_err("Fail openning file to be wrote");
            return -1;
          }
          next_tlv += filename_len + 2;
        }
      }
    }
    else if (packet[0] == PACK_END) {
      break;
    }
    else if (packet[0] == PACK_DATA && transmitting_data) {
      if (sequence_number%256 == packet[1]) {
        sequence_number++;
      }
      else if ((sequence_number-1)%256 == packet[1]) {
        continue;
      }
      else {
        log_err("Sequence of packets received is wrong");
        break;
      }

      int data_size = packet[2]*256 + packet[3];
      int bytes_written = write(fd, &packet[4], data_size);
      if (bytes_written < 0) {
        log_err("Failed to writing data bytes to the file");
        break;
      }

      new_filesize += bytes_written;
      log_progression(new_filesize, al.filesize, RECEIVER);
    }
    else {
      log_err("Not receiving a valid packet");
    }
  }
  
  if (close(fd) < 0) {
    log_err("Fail closing file wrote");
  }

  if (al.filesize != new_filesize) {
    log_msg("The file was received UNSUCCESSFULLY");
    return -1;
  }

  log_msg("The file was received SUCCESSFULLY");
  return 0;
}


int main(int argc, char** argv) {
  char port[12];
  if (parse_args(port, argc, argv) < 0) {
    printf("Usage:\tapp status port path\nstatus = {transmitter, receiver}\nport = {0, 10, 11}\n");
    return -1;
  }



  log_msg("Establishing connection");
  al.fd = llopen(port, al.status);
  if (al.fd == -1) {
    log_err("Unable to establishing connection");
    log_msg("Closing");
    return -1;
  }

  if (al.status == TRANSMITTER) {
    log_msg("Transfering data");
    transmitter();
  }
  else {
    log_msg("Receiving data");
    receiver();
  }
  
  log_msg("Ending connection");
  if (llclose(al.fd, al.status) == -1) {
    log_msg("Unable to ending connection");
    log_msg("Closing");
    return -1;
  }

  log_msg("Closing");
  return 0;
}
