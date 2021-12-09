#pragma once

#include "macros.h"


// Eficiency tests
#define EFFICIENCY_TEST 1
#define FER       0  // %
#define T_PROP    0 // ms


// ----------------- EFFICIENCY TEST -----------------------

/**
 * @brief Produce a random uchar with a probability of FER defined in macros_dl.h
 * 
 * @param byte Original byte
 * @param bcc Number of BCC where is generating the error
 * @return uchar Random byte with a probabbility of FER, otherwise returns the original byte
 */
uchar generate_error_BCC(uchar byte, int bcc);

void start_time();

double elapsed_time_ms();

void log_efficiency(int frame_size);


// ---------------------------------------------------------

// --------- LOG EFFICIENCY TEST -----------

/**
 * @brief Print a message to report a bcc error manually generated
 * 
 * @param bcc 1 to BCC_1 or 2 to BCC_2
 */
void log_bcc_error(int bcc);

void log_time_ms(double time_ms);

void log_datarate(double nbytes, double time_ms);

// -----------------------------------
