#include "auxiliar.h"
#include <stdio.h>
#include <string.h>


void print_frame(uchar* frame, int length) {
  for (int i = 0; i < length; i++) {
    printf(":%x", frame[i]);
  }
  printf("\n");
}

uchar get_BCC_1(uchar a, uchar b) {
  return a ^ b;
}

uchar get_BCC_2(uchar* data, int length) {
  uchar bcc2 = data[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ data[i];
  }
  return bcc2;
}


int byte_stuffing(uchar* frame, int length) {
  int finalLen = DATA_BEGIN;

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == FLAG) {
      frame[finalLen++] = ESCAPE;
      frame[finalLen++] = FLAG_STUFFING;
    }
    else if (aux[i] == ESCAPE) {
      frame[finalLen++] = ESCAPE;
      frame[finalLen++] = ESCAPE_STUFFING;
    }
    else {
      frame[finalLen++] = aux[i];
    }
  }
  frame[finalLen++] = FLAG;
  return finalLen;
}



int byte_destuffing(uchar* frame, int length) {
  int finalLen = DATA_BEGIN; 

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == ESCAPE) {
      if (aux[i+1] == FLAG_STUFFING) {
        frame[finalLen++] = FLAG;
        i++;
      }
      else if(aux[i+1] == ESCAPE_STUFFING) {
        frame[finalLen++] = ESCAPE;
        i++;
      }
    }
    else {
      frame[finalLen++] = aux[i];
    }
  }
  frame[finalLen++] = FLAG;
  return finalLen;
} 