#include <signal.h>
#include <stddef.h>
#include "log.h"
#include "alarm.h"

int n_sends, finish, send_frame;


void alarm_handler() {
  log_msg("Time out");
  send_frame = TRUE;
  n_sends++;
}

int set_alarm() {
  struct sigaction sa;
  sigset_t smask;

  if (sigemptyset(&smask) == -1) {
    log_err("sigemptyset() - setting alarm signal");
    return -1;
  }
      
  sa.sa_handler = alarm_handler;
  sa.sa_mask = smask;
  sa.sa_flags = 0;

  if (sigaction(SIGALRM, &sa, NULL) == -1) {
    log_err("sigaction() - setting alarm signal");
    return -1;
  }
  return 0;
}