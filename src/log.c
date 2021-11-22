#include <stdio.h>
#include "log.h"

void log_msg(char* s) {
  printf("MSG  -  %s\n", s);
}
void log_suc(char* s) {
  printf("SUC  -  %s\n", s);
}
void log_err(char* s) {
  printf("ERR  -  %s\n", s);
}