#include <stdlib.h>
#include <stdio.h>
#include "state_machine.h"
#include "aux.h"

State_machine* create_sm(uchar address, uchar* controls, int n_controls) {
  State_machine* sm = malloc(sizeof(State_machine));
  sm->state = START;
  sm->address = address;
  sm->controls = controls;
  sm->n_controls = n_controls;
  sm->frame_size = 0;
  
  return sm;
}

void destroy_sm(State_machine* sm) {
  free(sm);
}

int get_control(State_machine* sm, uchar byte) {
  for (int j = 0; j < sm->n_controls; j++) {
    if (sm->controls[j] == byte)
      return j;
  }
  return -1;
}

void event_handler_sm(State_machine* sm, uchar byte, uchar* frame, int frame_type) {
  static int i = 0;
  int n;

  switch (sm->state) {
    case START:
      i = 0;
      if (byte == FLAG) {
        sm->state = FLAG_RCV;
        frame[i++] = byte;
      }
      break;
    case FLAG_RCV:
      if (byte == FLAG) {
        i = 0;
        frame[i++] = byte;
      }
      else if (byte == sm->address) {
        sm->state = A_RCV;
        frame[i++] = byte;
      }
      else {
        sm->state = START;
        i = 0;
      }
      break;
    case A_RCV:
      if (byte == FLAG) {
        sm->state = FLAG_RCV;
        i = 0;
        frame[i++] = byte;      
      }
      else if ((n = get_control(sm, byte)) != -1) {
        sm->state = C_RCV;
        sm->control_chosen = n;
        frame[i++] = byte;
      }
      else {
        sm->state = START;
        i = 0;
      }
      break;
    case C_RCV:
      if (byte == FLAG) {
        sm->state = FLAG_RCV;
        i = 0;
        frame[i++] = byte;   
      }
      else if (byte == get_BCC_1(frame[ADDRS_BYTE], frame[CNTRL_BYTE])) {
        sm->state = BCC_OK;
        frame[i++] = byte;   
      }
      else {
        sm->state = START;
        i = 0;
      }
      break;
    case BCC_OK:
      if (byte == FLAG) {
        sm->state = STOP;
        frame[i++] = byte;
        sm->frame_size = i;
      }
      else {
        if (frame_type != INFORMATION) {
          sm->state = START;
        }
        else {
          frame[i++] = byte;
        }
      }
      break;
    default:
      break;
  }
}

