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
  uchar* wanted_controls;
  int   n_controls;
  int   control_chosen;
  int   frame_size;
} State_machine;

/**
 * @brief Create and initialize a state machine struct
 * 
 * @param address Address byte to be found
 * @param wanted_controls Set of possible control bytes to be found
 * @param n_controls Size of wanted_controls
 * @return State_machine* State machine to read frames
 */
State_machine* create_sm(uchar address, uchar* wanted_controls, int n_controls);
/**
 * @brief Free State machine
 * 
 * @param sm State machine to be free
 */
void destroy_sm(State_machine* sm);
/**
 * @brief Find a byte inside its expected control bytes
 * 
 * @param sm State machine
 * @param byte Control byte to be find in wanted_controls
 * @return int If byte exists in wanted_controls array, return its index, otherwise return -1
 */
int get_control(State_machine* sm, uchar byte);
/**
 * @brief Update the state machine according to its state and the byte given. Also fill frame accordingly.
 * 
 * @param sm State machine
 * @param byte Byte to deal with
 * @param frame Frame to be set
 * @param frame_type INFORMATION/SUPERVISION/UNNUMBERED frame
 */
void event_handler_sm(State_machine* sm, uchar byte, uchar* frame, int frame_type);

void process_start(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_flag_rcv(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_a_rcv(State_machine* sm, uchar byte, int* i, uchar* frame);
void process_c_rcv(State_machine* sm, uchar byte, int* i, uchar* frame, int frame_type);
void process_bcc_ok(State_machine* sm, uchar byte, int* i, uchar* frame, int frame_type);




