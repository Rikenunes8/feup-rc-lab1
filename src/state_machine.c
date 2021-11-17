#include <stdlib.h>
#include "state_machine.h"
#include "macros.h"

State_machine* create_sm(char address, char control, char bcc1) {
  State_machine* sm = malloc(sizeof(State_machine));
  sm->state = START;
  sm->address = address;
  sm->control = control;
  sm->bcc1 = bcc1;
  return sm;
}

void destroy_sm(State_machine* sm) {
  free(sm);
}

void change_state_sm(State_machine* sm, State state) {
  sm->state = state;
}

void event_handler_sm(State_machine* sm, char byte) {
  switch (sm->state) {
    case START:
      if (byte == FLAG) {
        change_state_sm(sm, FLAG_RCV);
      }
      break;
    case FLAG_RCV:
      if (byte == FLAG) {
        break;
      }
      else if (byte == sm->address) {
        change_state_sm(sm, A_RCV);
      }
      else {
        change_state_sm(sm, START);
      }
      break;
    case A_RCV:
      if (byte == FLAG) {
        change_state_sm(sm, FLAG_RCV);
      }
      else if (byte == sm->control) {
        change_state_sm(sm, C_RCV);
      }
      else {
        change_state_sm(sm, START);
      }
      break;
    case C_RCV:
      if (byte == FLAG) {
        change_state_sm(sm, FLAG_RCV);
      }
      else if (byte == sm->bcc1) {
        change_state_sm(sm, BCC_OK);
      }
      else {
        change_state_sm(sm, START);
      }
      break;
    case BCC_OK:
      if (byte == FLAG) {
        change_state_sm(sm, STOP);
      }
      else {
        change_state_sm(sm, START);
      }
      break;
  }
}

