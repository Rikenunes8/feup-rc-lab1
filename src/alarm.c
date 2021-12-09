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
  // signal(SIGALRM, alarm_handler);  // Could be used to replace the following commands and by setting VTIME of non-canonical openning of serial port as not 0
                                      // However the use of sigaction is used instead of singal since it interrupts the read system call when a alarm is called, consuming less of the processor

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