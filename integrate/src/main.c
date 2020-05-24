#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>           // sqrt
#include <string.h>         // str*, malloc
#include <sys/socket.h>     // socket, accept, listen
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <errno.h>

#include "cat.h"
#include "commons.h"
#include "slave.h"

#define SLAVE_MAX   10
#define MASTER      1
#define SLAVE       2

#define HELP_PATH "assets/help.txt"

#define UNDEF       ~0

// Mater routine
void master(int port, int slaves, double dx);

int calc_integ(double (*f)(double), double min, double max, double dx, double *result);
void print_usage();
void bad_usage();

void print_usage() {
    cat(HELP_PATH);
    exit(0);
}

void bad_usage() {
    fprintf(stderr, "Bad usage.\nSee 'integ --help'.\n");
    exit(1);
}

double func (double x) {
    return sqrt(10000.0 - x*x);
}

void master(int port, int slaves, double dx) {
    int TRUE = true;                  //
    int srv;                          // Server socket
    int sock;                         // Response socket
    struct pollfd sfd[SLAVE_MAX * 2]; // Set of sockets
    int i;                            // Socket set index
    int deliv;                        // Number of delivered calculus
    int chunk;                        // Number of chunks of calculus
    double sum;                       // Slaves' instegrate sum
    double size;                      // How much each slave will calc
    double start;                     // The interval begin
    pack_t pack;                      // Package of the communication
    struct sockaddr_in address;       // The server socket address
    int addrlen;                      // Size of address struct
    int j;                            // Loop iterator
    ////////////////////////////////////
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

int main(int argc, char const *argv[]) {
    char host[64];                  // the master host
    strcpy(host, DEFAULT_MASTER);
    int port = DEFAULT_PORT;        // the port master listen to
    int number = DEFAULT_SLAVES;    // the number of slaves master expects
    double step = DEFAULT_STEP;     // the step size for the integration
    int i;                          // the loop iterator
    int err;                        // the error checker
    int type = -1;                  // the type of this process (MASTER | SLAVE)
    // START arguments verification
    if (argc < 2) {
        bad_usage();
    }
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage();
    } else if (strcmp(argv[1], "master") == 0) {
        type = MASTER;
    } else if (strcmp(argv[1], "slave") == 0) {
        type = SLAVE;
    } else {
        fprintf(stderr, "Error: Neighter 'master' or 'slave'.\n");
        bad_usage();
    }
    i = 2;
    while (i < argc) {
        // -n | --number option
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number") == 0) {
            if (type == SLAVE) {
                fprintf(stderr, "Warning: useless option '%s' for type slave\n", argv[i]);
            }
            i++;
            if (i == argc) {
                fprintf(stderr, "Error: no number defined for '%s'\n", argv[i-1]);
                bad_usage();
            }
            err = sscanf(argv[i], "%d", &number);
            if (err == 0) {
                fprintf(stderr, "Error: '%s' is not an integer number.\n", argv[i]);
                bad_usage();
            }
            i++;
            continue;
        }
        // -s | --step option
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--step") == 0) {
            if (type == SLAVE) {
                fprintf(stderr, "Warning: useless option '%s' for type slave\n", argv[i]);
            }
            i++;
            if (i == argc) {
                fprintf(stderr, "Error: no value defined for '%s'\n", argv[i-1]);
                bad_usage();
            }
            err = sscanf(argv[i], "%lf", &step);
            if (err == 0) {
                fprintf(stderr, "Error: '%s' is not a number.\n", argv[i]);
                bad_usage();
            }
            i++;
            continue;
        }
        // -m | --master option
        else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--master") == 0) {
            if (type == MASTER) {
                fprintf(stderr, "Warning: useless option '%s' for type master\n", argv[i]);
            }
            i++;
            if (i == argc) {
                fprintf(stderr, "Error: no host defined for '%s'\n", argv[i-1]);
                bad_usage();
            }
            strcpy(host, argv[i]);
            i++;
            continue;
        }
        // -p | --port option
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            i++;
            if (i == argc) {
                fprintf(stderr, "Error: no port defined for '%s'\n", argv[i-1]);
                bad_usage();
            }
            err = sscanf(argv[i], "%d", &port);
            if (err == 0) {
                fprintf(stderr, "Error: '%s' is not a number.\n", argv[i]);
                bad_usage();
            }
            i++;
            continue;
        } else {
            fprintf(stderr, "Error: invalid option '%s'.\n", argv[i]);
            bad_usage();
        }
    }
    // END arguments verification
    if (type == MASTER) {
        master(port, number, step);
    } else { // type = SLAVE
        slave_set_function(func);
        slave(host, port);
    }
    
    return 0;
}
