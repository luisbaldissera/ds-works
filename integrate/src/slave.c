#include "slave.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>      // EPROTO
#include <sys/socket.h> // socket, connect
#include <unistd.h>     // close
#include <netdb.h>      // getaddrinfo
#include "commons.h"    // pack_send, pack_recv, ERR_EXIT, THROW, print
#include "integrate.h"  // integrate

// The function the will be integrated
double (*slave_func) (double) = NULL;

void slave_set_function(double (*f)(double)) {
    slave_func = f;
}

void slave(char const *host, int port) {
    int sock;                       // Socket to connect to master
    int err;                        // Error auxiliar
    struct addrinfo *info;          // Info for name resolution
    pack_t pack;                    // The packet sctructure for communication
    //////////////////////////////////

    // Open slave socket that will be used to connect to master
    print("Opening SLAVE socket...\n");
    ERR_EXIT(sock = socket(AF_INET, SOCK_STREAM, 0)); // IPv4 / TCP

    print("Resolving master hostname...\n");
    ERR_EXIT(getaddrinfo(host, NULL, NULL, &info));
    ((struct sockaddr_in *) info->ai_addr)->sin_port = htons(port);
    print("Connecting to '%s' in port %d...\n", host, port);
    ERR_EXIT(connect(sock, info->ai_addr, sizeof(struct sockaddr)));
    print("Connected!\n");
    // Start master session
    bzero(&pack, sizeof(pack));
    pack.type = CALC_REQUEST;
    print("Requesting calculus to master...\n");
    // Send a calculus request
    pack_send(sock, &pack);
    // Waits for receiving a response to its request
    pack_recv(sock, &pack);
    switch (pack.type) {
        case CALC_RESPONSE:
            print("Calculating from %g to %g, with dx = %g...\n", pack.min, pack.max, pack.dx_res);
            pack.dx_res = integrate(slave_func, pack.min, pack.max, pack.dx_res);	// Calculate integral
            pack.type = CALC_RESULT;
            print("Result: %g\n", pack.dx_res);
            print("Sending to master...\n");
            pack_send(sock, &pack);
            break;
        case CALC_NONEED:
            print("Master doesn't need me. Good bye!\n");
            break;
        default:		// Package was not correctly understood
            THROW(EPROTO);  	// Prints "Protocol Error" and exit
    }

    close(sock);
    print("My service is done! Goodbye!\n");
}
