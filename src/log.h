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