#pragma once

#include "macros.h"

/**
 * @brief Handle when an alarm signal is received
 * 
 */
void alarm_handler();
/**
 * @brief Set alarm's signal behaviour
 * 
 * @return int 0 on success, -1 on failure
 */
int set_alarm();
