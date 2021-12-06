#pragma once

#include "macros.h"


// Eficiency tests
#define EFFICIENCY_TEST 1
#define FER       0
#define T_PROP    0


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

// ---------------------------------------------------------
