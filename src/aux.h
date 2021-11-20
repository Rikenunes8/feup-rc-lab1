#pragma once
#include "macros.h"

// -------- AUXILIAR FUNCTIONS -----------
uchar get_BCC_1(uchar a, uchar b);

uchar get_BCC_2(uchar* data, int length);

// --------------------------------------------

int byteStuffing(char* frame, int length);

int byteDestuffing(char* frame, int length);

void print_frame(uchar* frame, int length);