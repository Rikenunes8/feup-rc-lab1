#pragma once

#include "macros.h"


// Eficiency tests
#define EFFICIENCY_TEST 1
#define FER     0 // %
#define T_PROP  0 // us

// ----------------- EFFICIENCY TEST -----------------------
/**
 * @brief Set stats_t structure with the values predefined
 * 
 * @param baudrate Baudrate of the serial port
 * @param max_size Frame with no stuffing size
 */
void init_stats(int baudrate, int max_size);

/**
 * @brief Produce a random uchar with a probability of FER defined in macros_dl.h
 * 
 * @param byte Original byte
 * @param bcc Number of BCC where is generating the error
 * @return uchar Random byte with a probabbility of FER, otherwise returns the original byte
 */
uchar generate_error_BCC(uchar byte, int bcc);

/**
 * @brief Set test_time with the current time
 * 
 */
void start_time();
/**
 * @brief Calculate the elapsed time between test_time and the current time
 * 
 * @return double Elapsed time in mili seconds (ms)
 */
double elapsed_time_ms();
/**
 * @brief Set the remains attributes of the stats_s with the values obtained
 * 
 * @param size Size in bytes to measure the efficiency
 */
void efficiency(int size);


// --------- LOG EFFICIENCY TEST -----------

/**
 * @brief Print a message to report a bcc error manually generated
 * 
 * @param bcc 1 to BCC_1 or 2 to BCC_2
 */
void log_bcc_error(int bcc);
/**
 * @brief Print stats to stderr
 * 
 */
void log_stats();

