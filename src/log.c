#include <stdio.h>
#include "log.h"

void log_msg(char* s) {
  printf("MSG  -  %s\n", s);
}

void log_err(char* s) {
  printf("ERR  -  %s\n", s);
}

void log_sent(char* type, int n) {
  printf("SNT  -  %s", type);
  if (n != -1)
    printf("-%d", n);
  printf("\n");
}

void log_rcvd(char* type, int n) {
  printf("RCV  -  %s", type);
  if (n != -1)
    printf("-%d", n);
  printf("\n");
}

void log_progression(long part, long total, int status) {
  if (status == 0) {
    printf("MSG  -  %ld / %ld bytes sent\n", part, total);
  }
  else {
    printf("MSG  -  %ld / %ld bytes received\n", part, total);
  }

}