#include <stdio.h>
#include <signal.h>
#include "alarm.h"

void alarm_handler() {
  printf("Time out # %d\n", count);
  send = TRUE;
  count++;
}

void set_alarm() {
  struct sigaction sa;
  sigset_t smask;

  if (sigemptyset(&smask) == -1) {
    fprintf(stderr, "Error in sigemptyset() while setting alarm signal\n");
    return;
  }
      
  sa.sa_handler = alarm_handler;
  sa.sa_mask = smask;
  sa.sa_flags = 0;

  sigaction(SIGALRM, &sa, NULL);
  
  siginterrupt(SIGALRM, 1);
}