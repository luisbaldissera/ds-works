#include "commons.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

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

