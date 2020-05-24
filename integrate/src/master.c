#include "master.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         // close
#include <sys/socket.h>     // socket, bind, listen
#include <netdb.h>          // sockaddr*
#include <string.h>         // bzero
#include <poll.h>           // poll
#include <stdbool.h>        // true, false
#include "commons.h"        // pack_send, pack_recv

void master(int port, int slaves, double dx) {
    int TRUE = true;            //
    int srv;                    // Server socket
    int sock;                   // Response socket
    struct pollfd sfd[16];      // Set of sockets
    int i;                      // Socket set index
    int deliv;                  // Number of delivered calculus
    int chunk;                  // Number of chunks of calculus
    double sum;                 // Slaves' instegrate sum
    double size;                // How much each slave will calc
    double start;               // The interval begin
    pack_t pack;                // Package of the communication
    struct sockaddr_in address; // The server socket address
    int addrlen;                // Size of address struct
    int j;                      // Loop iterator
    // Initialize variables
    chunk = slaves;
    deliv = 0;
    sum = 0;
    start = 0.0;
    size = 100.0 / slaves;
    addrlen = sizeof(address);
    // Open Server socket
    printf("Opening MASTER socket...\n");
    ERR_EXIT(srv = socket(AF_INET, SOCK_STREAM, 0));
    ERR_EXIT(setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &(TRUE), sizeof(true)));
    // Configure address
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // localhost
    address.sin_port = htons(port);         // set port
    // Attach server socket to port
    printf("Attaching socket to port %d...\n", port);
    ERR_EXIT(bind(srv, (struct sockaddr *) &address, sizeof(address)));
    // Start listening (incoming connections up to 10)
    ERR_EXIT(listen(srv, 5));
    printf("Listening on port %d...\n", port);
    // intialize pool
    bzero(sfd, sizeof(sfd));
    sfd[0].fd = srv;
    sfd[0].events = POLLIN;
    i = 1;
    // Starts server routine
    while (deliv > 0 || chunk > 0) {
        poll(sfd, i, 100);
        for (j = 0; j < i; j++) {
            if (sfd[j].revents & POLLIN) {
                if (sfd[j].fd == srv) { // Server was triggered (new client)
                    sock = accept(srv, NULL, NULL);
                    sfd[i].fd = sock;
                    sfd[i].events = POLLIN;
                    sfd[i].revents = 0;
                    i++;
                    printf("New slave connected.\n");
                } else { // Client is speaking
                    bzero(&pack, sizeof(pack));
                    pack_recv(sfd[j].fd, &pack);
                    switch (pack.type) {
                        case CALC_REQUEST:
                            printf("Slave requested for calculus\n");
                            bzero(&pack, sizeof(pack));
                            if (chunk == 0) {
                                pack.type = CALC_NONEED;
                                printf("No need for more calculus.\n");
                                pack_send(sfd[j].fd, &pack);
                                close(sfd[j].fd);
                                sfd[j].fd = -1; // ignore socket
                            } else {
                                pack.type = CALC_RESPONSE;
                                pack.min = start;
                                start += size;
                                pack.max = start;
                                pack.dx_res = dx;
                                pack_send(sfd[j].fd, &pack);
                                chunk--;
                                deliv++;
                                printf("Calculates from %g to %g with dx = %g\n", pack.min, pack.max, pack.dx_res);
                            }
                            break;
                        case CALC_RESULT:
                            printf("Slave ended calculus.\n");
                            printf(":: from %g to %g. Result: %g\n", pack.min, pack.max, pack.dx_res);
                            sum += pack.dx_res;
                            deliv--;
                            close(sfd[j].fd);
                            sfd[j].fd = -1;
                            break;
                        case CALC_FAIL:
                            printf("Slave calculus failed.\n");
                            chunk++;
                            deliv--;
                            bzero(&pack, sizeof(pack));
                            pack.type = CALC_NONEED;
                            pack_send(sfd[j].fd, &pack);
                            close(sfd[j].fd);
                            sfd[j].fd = -1;
                            break;
                        default:
                            THROW(EPROTO);
                    }
                }
            }
        }
    }
    printf("Result: %g\n", sum);
    // Close opened sockets
    for (j = 0; j < i; j++) {
        if (sfd[j].fd != -1) {
            close(sfd[j].fd);
        }
    }
}
