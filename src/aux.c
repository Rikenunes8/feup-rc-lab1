#include "aux.h"
#include <stdio.h>


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


int byteStuffing(uchar* frame, int length) {
  int fullLen = length + 6; int finalLen = DATA_BEGIN;

  char aux[fullLen];

  for (int i = DATA_BEGIN; i < fullLen; i++) {
    aux[i] = frame[i];
  }

  for (int i = DATA_BEGIN; i < fullLen-1; i++) {
    if (aux[i] == FLAG) {
      frame[finalLen] = ESCAPE;
      frame[finalLen+1] = FLAG_STUFFING;
      finalLen = finalLen + 2;
    }

    else if (aux [i] == ESCAPE) {
      frame[finalLen] = ESCAPE;
      frame[finalLen+1] = ESCAPE_STUFFING;
      finalLen = finalLen + 2;
    }

    else {
      finalLen++;
    }
  }

  return finalLen;
}



int byteDestuffing(uchar* frame, int length) {
  char aux[length + 5]; //bcc2 is included in stuffing
  int finalLen = DATA_BEGIN; int fullLen = length + 5;

  for (int i = DATA_BEGIN; i < fullLen; i++) {
    aux[i] = frame[i];
  }


  for (int i = DATA_BEGIN; i < fullLen; i++, finalLen++) {
    if (aux[i+1] == FLAG_STUFFING) {
      frame[finalLen] = FLAG;
      i++;
    }

    else if(aux[i+1] == ESCAPE_STUFFING) {
      frame[finalLen] = ESCAPE;
      i++;
    }

    else {
      frame[finalLen] = aux[i];
    }

  }

  return finalLen;
} 