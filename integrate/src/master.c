#include "master.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>         // close
#include <sys/socket.h>     // socket, bind, listen
#include <netdb.h>          // sockaddr*
#include <string.h>         // bzero
#include <poll.h>           // poll, pollfd
#include <stdbool.h>        // true, false
#include <time.h>           // clock
#include "commons.h"        // pack_send, pack_recv, ERR_EXIT, THROW, print

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
    double texec;               // Time of executions
    // Initialize variables
    chunk = slaves;
    deliv = 0;
    sum = 0;
    start = 0.0;
    size = 100.0 / slaves;
    addrlen = sizeof(address);
    // Open Server socket
    print("Opening MASTER socket...\n");
    ERR_EXIT(srv = socket(AF_INET, SOCK_STREAM, 0));
    ERR_EXIT(setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &(TRUE), sizeof(true)));
    // Configure address
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // localhost
    address.sin_port = htons(port);         // set port
    // Attach server socket to port
    print("Attaching socket to port %d...\n", port);
    ERR_EXIT(bind(srv, (struct sockaddr *) &address, sizeof(address)));
    // Start listening (incoming connections up to 10)
    ERR_EXIT(listen(srv, 5));
    print("Listening on port %d...\n", port);
    // intialize pool with server socket
    bzero(sfd, sizeof(sfd));
    sfd[0].fd = srv;
    sfd[0].events = POLLIN;
    i = 1;
    // Starts server routine
    while (deliv > 0 || chunk > 0) {
        poll(sfd, i, 100);		// Blocks execution util something happens in any socket
        for (j = 0; j < i; j++) {
            if (sfd[j].revents & POLLIN) {	// Check if socket[j] received input
                if (sfd[j].fd == srv) { 		// Server was triggered (new client)
                    if (i == 1) tic(); // Starts counting time in the first slave connection
                    sock = accept(srv, NULL, NULL);	// Accept new client (slave)
                    sfd[i].fd = sock;			// Adds client to be listened
                    sfd[i].events = POLLIN;
                    sfd[i].revents = 0;
                    i++;
                    print("New slave connected.\n");
                } else { 				// Client is speaking
                    bzero(&pack, sizeof(pack));
                    pack_recv(sfd[j].fd, &pack);	// Read socket
                    switch (pack.type) {
                        case CALC_REQUEST:		// Slave request calculus
                            print("Slave requested calculus\n");
                            bzero(&pack, sizeof(pack));
                            if (chunk == 0) {		// All calculus was delivered
                                pack.type = CALC_NONEED;
                                print("No need for more calculus.\n");
                                pack_send(sfd[j].fd, &pack);
                                close(sfd[j].fd);
                                sfd[j].fd = -1;		// Ignore socket events
                            } else {			// Answer slave with calculus interval
                                pack.type = CALC_RESPONSE;
                                pack.min = start;
                                start += size;
                                pack.max = start;
                                pack.dx_res = dx;
                                pack_send(sfd[j].fd, &pack);
                                chunk--;
                                deliv++;
                                print("Calculates from %g to %g with dx = %g\n", pack.min, pack.max, pack.dx_res);
                            }
                            break;
                        case CALC_RESULT:		// Slave endded calculus and returned it
                            print("Slave ended calculus.\n");
                            print(":: from %g to %g. Result: %g\n", pack.min, pack.max, pack.dx_res);
                            sum += pack.dx_res;
                            deliv--;
                            close(sfd[j].fd);
                            sfd[j].fd = -1;
                            break;
                        case CALC_FAIL:			// Slave calculus failed
                            print("Slave calculus failed.\n");
                            chunk++;
                            deliv--;
                            bzero(&pack, sizeof(pack));
                            pack.type = CALC_NONEED;
                            pack_send(sfd[j].fd, &pack);
                            close(sfd[j].fd);
                            sfd[j].fd = -1;
                            break;
                        default:			// Message received was not correctly understood
                            THROW(EPROTO);		// Show "Protocol Error" and exits
                    }
                }
            }
        }
    }
    // Ends timer
    texec = toc();
    print("Result: %g\n", sum);
    printf("Time: %g ms\n", texec);
    // Close opened sockets
    for (j = 0; j < i; j++) {
        if (sfd[j].fd != -1) {
            close(sfd[j].fd);
        }
    }
}
