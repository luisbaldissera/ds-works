#include "commons.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdarg.h> // ..., va_list

int _can_print = 1; // State variable to define printing
struct timeval start;     // The timer reset clock

int print(char const *fmt, ...) {
    int ret = 0;
    if (_can_print) {
        va_list args;
        va_start(args, fmt);
        ret = vprintf(fmt, args);
        va_end(args);
    }
    return ret;
}

void set_print(int set) {
    _can_print = set;
}

void tic() {
    gettimeofday(&start, NULL);
}

double toc() {
    struct timeval end;
    long usec;
    gettimeofday(&end, NULL);
    usec = ((end.tv_sec - start.tv_sec) * 1000000) + (end.tv_usec - start.tv_usec);
    return ((double)usec) / 1000;
}
