#pragma once

/**
 * @brief Print a message for interesting content
 * 
 * @param s Personalized text
 */
void log_msg(char* s);
/**
 * @brief Print a message for an error occured
 * 
 * @param s Personalized text
 */
void log_err(char* s);
/**
 * @brief Print a message for a frame transmitted
 * 
 * @param type Personalized text
 * @param n -1 when not necessary otherwise add the number to the message
 */
void log_sent(char* type, int n);
/**
 * @brief Print a message for a frame received
 * 
 * @param type Personalized text
 * @param n -1 when not necessary otherwise add the number to the message
 */
void log_rcvd(char* type, int n);
/**
 * @brief Print a message in format "part / total bytes"
 * 
 * @param part Current progression of bytes sent/received
 * @param total Total bytes to be sent/received
 * @param status 0 to transmitter or 1 to receiver
 */
void log_progression(long part, long total, int status);


// --------- EFFICIENCY TEST -----------

/**
 * @brief Print a message to report a bcc error manually generated
 * 
 * @param bcc 1 to BCC_1 or 2 to BCC_2
 */
void log_bcc_error(int bcc);

void log_time_ms(double time_ms);

void log_datarate(double nbytes, double time_ms);

// -----------------------------------
