#include <stdio.h>
#include "log.h"

void log_msg(char* s) {
  printf("MSG  -  %s\n", s);
}

void log_err(char* s) {
  printf("ERROR  -  %s\n", s);
}

void log_sent(char* type, int n) {
  printf("SNT  -  FRAME %s", type);
  if (n != -1)
    printf("-%d\n", n);
  else
    printf("\n");
}

void log_rcvd(char* type, int n) {
  printf("RCV  -  FRAME %s", type);
  if (n != -1)
    printf("-%d\n", n);
  else
    printf("\n");
}