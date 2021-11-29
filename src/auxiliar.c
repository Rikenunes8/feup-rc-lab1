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
  int final_len = DATA_BEGIN;

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == FLAG) {
      frame[final_len++] = ESCAPE;
      frame[final_len++] = FLAG_STUFFING;
    }
    else if (aux[i] == ESCAPE) {
      frame[final_len++] = ESCAPE;
      frame[final_len++] = ESCAPE_STUFFING;
    }
    else {
      frame[final_len++] = aux[i];
    }
  }
  frame[final_len++] = FLAG;
  return final_len;
}

int byte_destuffing(uchar* frame, int length) {
  int final_len = DATA_BEGIN; 

  uchar aux[length];
  memcpy(aux, frame+DATA_BEGIN, length-SU_SIZE);

  for (int i = 0; i < length-SU_SIZE; i++) {
    if (aux[i] == ESCAPE) {
      if (aux[i+1] == FLAG_STUFFING) {
        frame[final_len++] = FLAG;
        i++;
      }
      else if(aux[i+1] == ESCAPE_STUFFING) {
        frame[final_len++] = ESCAPE;
        i++;
      }
    }
    else {
      frame[final_len++] = aux[i];
    }
  }
  frame[final_len++] = FLAG;
  return final_len;
} 