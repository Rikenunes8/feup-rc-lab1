#pragma once


int open_non_canonical(char* file, struct termios* oldtio, int vtime, int vmin);

int close_non_canonical(int fd, struct termios* oldtio);