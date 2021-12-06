#include "efficiency.h"

#define _POSIX_C_SOURCE 199309L

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static struct timespec test_time; // EFFICIENCY TEST

// static clock_t test_time;

//static struct timeval test_time;


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
  
  // test_time = clock();
  
  //gettimeofday(&test_time, NULL);
}

double elapsed_time_ms() {
  struct timespec current_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  double elapsed = (current_time.tv_sec-test_time.tv_sec)*1000 + ((current_time.tv_nsec-test_time.tv_nsec)/10e6);
  
  //test_time = clock() - test_time;
  //double clocks_per_ms = CLOCKS_PER_SEC*0.001;
  //double elapsed = (double)test_time/clocks_per_ms;
  
  //struct timeval end;
  //gettimeofday(&end, NULL);
  //double elapsed = (end.tv_sec-test_time.tv_sec)*10e3 + (end.tv_usec-test_time.tv_usec)*10e-3;

  return elapsed;
}

// ---------------------------------------------------------