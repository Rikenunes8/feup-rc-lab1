#include "efficiency.h"

#define _POSIX_C_SOURCE 199309L

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>


typedef struct {
  int fer;
  int t_prop;
  int baudrate;
  int max_size;
  double datarate;
  double s;

  double bcc1_err;
  double bcc2_err;
} stats_t;

static struct timespec test_time; 
static stats_t stats;

// ----------------- EFFICIENCY TEST -----------------------

void init_stats(int baudrate, int max_size) {
  srand(time(NULL));
  stats.fer = FER;
  stats.bcc1_err = 3.0 / (double)(max_size+6) * FER; // Approximate probability of an error in bcc_1 given FER
  stats.bcc2_err = (double)max_size / (double)(max_size+6) * FER; // Approximate probability of an error in bcc_2 given FER
  stats.t_prop = T_PROP;
  stats.baudrate = baudrate;
  stats.max_size = max_size;
}

uchar generate_error_BCC(uchar byte, int bcc) {
  double prob = (rand()%10001)/100.0;
  if ((bcc == 1 && prob <= stats.bcc1_err) || (bcc == 2 && prob <= stats.bcc2_err)) {
    log_bcc_error(bcc);
    byte = byte ^ 0x01;
  }
  return byte;
}

void start_time() {
  clock_gettime(CLOCK_MONOTONIC, &test_time);
}

double elapsed_time_ms() {
  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  double elapsed = (current_time.tv_sec-test_time.tv_sec)*1000 + ((current_time.tv_nsec-test_time.tv_nsec)/10e6);

  return elapsed;
}

void efficiency(int size) {
  double time_ms = elapsed_time_ms();
  stats.datarate = (size)*8 / (time_ms/1000);
  stats.s = stats.datarate / (double)stats.baudrate;
  log_stats();
}



// --------- LOG EFFICIENCY TEST -----------

void log_bcc_error(int bcc) {
  char text[30];
  sprintf(text, "Generate BCC_%d error", bcc);
  log_msg(text);
}

void log_stats() {
  fprintf(stderr, "FER: %2d %%  T_prop: %1d ms  Size: %4d B  C: %5d Bits/s  R: %5d Bits/s  S: %2.0f %%\n", 
          stats.fer, (int)(stats.t_prop*10e-3), stats.max_size, stats.baudrate, (int)stats.datarate, stats.s*100);
}



