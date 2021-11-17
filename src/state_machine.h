#pragma once

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
  char address;
  char control;
  char bcc1;
} State_machine;

State_machine* create_sm(char address, char control, char bcc1);
void destroy_sm();
void change_state_sm(State_machine* sm, State state);
void event_handler_sm(State_machine* sm, char byte);