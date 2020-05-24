#include "commons.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void pack_send(int sock, pack_t *src) {
    static char buffer[128];
    sprintf(buffer, "%d %g %g %g",
            src->type,
            src->min,
            src->max,
            src->dx_res);
    send(sock, buffer, strlen(buffer), 0);
}

void pack_recv(int sock, pack_t *dest) {
    static char buffer[128];
    bzero(buffer, sizeof(buffer));
    recv(sock, buffer, 128, 0);
    sscanf(buffer, "%d %lf %lf %lf",
            &(dest->type),
            &(dest->min),
            &(dest->max),
            &(dest->dx_res));
}

