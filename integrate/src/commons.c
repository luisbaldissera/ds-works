#include "commons.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdarg.h> // ..., va_list

int _can_print = 1; // State variable to define printing
struct timeval start;     // The timer reset clock

void pack_send(int sock, pack_t *src) {
    static char buffer[128];		// Buffer for communication
    sprintf(buffer, "%d %g %g %g",	// Transform package in string
            src->type,
            src->min,
            src->max,
            src->dx_res);
    send(sock, buffer, strlen(buffer), 0); // Send string over socket
}

void pack_recv(int sock, pack_t *dest) {
    static char buffer[128];		// Buffer for communication
    bzero(buffer, sizeof(buffer));	// Reset buffer for receive new data
    recv(sock, buffer, 128, 0);		// Receive string
    sscanf(buffer, "%d %lf %lf %lf",	// Transform string in package
            &(dest->type),
            &(dest->min),
            &(dest->max),
            &(dest->dx_res));
}

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
