#ifndef ALARM_H
#define ALARM_H

#include "macros.h"

extern int count, allgood, send;

void alarm_handler();
void set_alarm();

#endif