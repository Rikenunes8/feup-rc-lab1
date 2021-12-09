#include "efficiency.h"

#define _POSIX_C_SOURCE 199309L

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>

static struct timespec test_time; // EFFICIENCY TEST

// ----------------- EFFICIENCY TEST -----------------------

uchar generate_error_BCC(uchar byte, int bcc) {
  int prob = (rand()%100) + 1;
  if (prob <= FER) {
    log_bcc_error(bcc);
    byte = rand()%256;
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

void log_efficiency(int frame_size) {
  double ms = elapsed_time_ms(); 
  log_time_ms(ms); 
  log_datarate(frame_size, ms);
}

// ---------------------------------------------------------

// --------- LOG EFFICIENCY TEST -----------

void log_bcc_error(int bcc) {
  char text[30];
  sprintf(text, "Generate BCC_%d error", bcc);
  log_msg(text);
}

void log_time_ms(double time_ms) {
  fprintf(stderr, "TIME -  %f ms\n", time_ms);
}

void log_datarate(double nbytes, double time_ms) {
  double bits = (nbytes) / (time_ms/1000);
  fprintf(stderr, "R    -  %0.1f bits/s \n", bits);
}

// -----------------------------------