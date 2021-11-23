#pragma once

typedef struct {
  int fd; /*Descritor correspondente à porta série*/
  int status; /*TRANSMITTER | RECEIVER*/
  char filename[256];
} Application_layer;

int parse_args(char* port, int argc, char** argv);
