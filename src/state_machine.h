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
int get_control(State_machine* sm, uchar byte);
void event_handler_sm(State_machine* sm, uchar byte, uchar* frame, int frame_type);

void process_start(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_flag_rcv(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_a_rcv(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_c_rcv(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_bcc_ok(State_machine* sm, uchar byte, int* i, uchar* frame, int frame_type);




