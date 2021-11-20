#pragma once

#include "macros.h"

typedef enum {
  START,
  FLAG_RCV,
  A_RCV,
  C_RCV,
  BCC_OK,
  STOP
} State;

typedef struct {
  State state;
  uchar  address;
  uchar* controls;
  int   n_controls;
  int   control_chosen;
  int   frame_size;
} State_machine;

State_machine* create_sm(uchar address, uchar* controls, int n_controls);
void destroy_sm();
void event_handler_sm(State_machine* sm, uchar byte, uchar* frame, int frame_type);
int get_control(State_machine* sm, uchar byte);