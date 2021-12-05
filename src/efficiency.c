#include "efficiency.h"

#define _POSIX_C_SOURCE 199309L

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static struct timespec mytime; // EFFICIENCY TEST


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
  clock_gettime(CLOCK_MONOTONIC, &mytime);
}

double ellapsed_time_ms() {
  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  double elapsed = (current_time.tv_sec-mytime.tv_sec)*1000+((current_time.tv_nsec-mytime.tv_nsec)/10e6);
  return elapsed;
}

// ---------------------------------------------------------