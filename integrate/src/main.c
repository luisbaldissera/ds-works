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

#define SLAVE_MAX   10
#define MASTER      1
#define SLAVE       2

#define X_IDX       0
#define XDX_IDX     1

// Macros to handle errors set with 'errno'
#define ERR_EXIT(cmd) do{if((int)(cmd)<(0)){fprintf(stderr,"Error (%s:%d): %s\n",__FILE__,__LINE__,strerror(errno));exit(EXIT_FAILURE);}}while(0)
#define THROW(err) ERR_EXIT(-(errno = (err)))

// Calculus request [slave -> master]
// package: [ 1 ]
#define CALC_REQUEST    1
// Calculus no needed [master -> slave]
// package: [ 2 ]
#define CALC_NONEED     2
// Calculus response [master -> slave]
// package: [ 2 min max ]
#define CALC_RESPONSE   3
// Calculus result [slave -> master]
// package [ 3 min max result ]
#define CALC_RESULT     4
// Calculus failed [slave -> master]
// package [ 5 min max ]
#define CALC_FAIL       5

#define HELP_PATH "assets/help.txt"

#define UNDEF       ~0

typedef struct {
    int type;
    double min;
    double max;
    double dx_res;
} pack_t;

extern int errno;

// Mater routine
void master(int port, int slaves, double dx);
// Slave routine
void slave(int port, char const *host, int reiter);
// Transform package data in string
int pack2str(pack_t *pack, char *str);
// Transform string in package data
int str2pack(char *str, pack_t *pack);
// Write package data to a socket
void pack_send(int sock, pack_t *pack);
// Read package data from a socket
void pack_recv(int sock, pack_t *pack);
// A cat for files
void cat(char const *fname);

int calc_integ(double (*f)(double), double min, double max, double dx, double *result);
void print_usage();
void bad_usage();

void cat(char const *fname) {
    int fd;
    int size = 512;
    char buffer[512];
    fd = open(fname, O_RDONLY);
    while (size == 512) {
        size = read(fd, buffer, 512);
        write(1, buffer, size);
    }
    close(fd);
}

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

int pack2str(pack_t *pack, char *str) {
    return sprintf(str, "%d %g %g %g", pack->type, pack->min, pack->max, pack->dx_res);
}

int str2pack(char *str, pack_t *pack) {
    return sscanf(str, "%d %lf %lf %lf", &(pack->type), &(pack->min), &(pack->max), &(pack->dx_res)) - 4;
}

void pack_send(int sock, pack_t *pack) {
    char buff[512];
    pack2str(pack, buff);
    send(sock, buff, strlen(buff), 0);
    printf(">[%s] ", buff);
}

void pack_recv(int sock, pack_t *pack) {
    char buff[254];
    bzero(buff, sizeof(buff));
    recv(sock, buff, 254, 0);
    printf("<[%s] ", buff);
    str2pack(buff, pack);
}

int calc_integ(double (*f)(double), double min, double max, double dx, double *result) {
    double cache = UNDEF;           // Cache to avoid repeated caclulus
    double trapeze = UNDEF;         // Trapeze area
    double const fac = dx / 2.0;    // Trapeze factor multiplication
    double x;                       // The X axe iterator
    *result = 0;
    cache = f(min);
    x = min + dx;
    while (x <= max) {
        trapeze = cache;
        cache = f(x);
        trapeze = (trapeze + cache) * fac;
        *result += trapeze;
        x += dx;
    }
    return EXIT_SUCCESS;
}

void master(int port, int slaves, double dx) {
    int TRUE = true;                  //
    int ended = 0;                    //
    int srv;                          // Server socket
    int sock;                         // Response socket
    int deliv;                        // Number of delivered calculus
    int chunk;                        // Number of chunks of calculus
    double sum;                       // Slaves' instegrate sum
    double size;                      // How much each slave will calc
    double start;                     // The interval begin
    pack_t pack;                      // Package of the communication
    struct sockaddr_in address;       // The server socket address
    int addrlen;                      // Size of address struct
    struct pollfd sfd[SLAVE_MAX * 2]; // Set of sockets
    int i;                            // Socket set index
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
        j = poll(sfd, i, 100);
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

void slave(int port, char const *host, int reiter) {
    int sock;                       // Socket to connect to master
    int err;                        // Error auxiliar
    struct sockaddr_in address;     // Server address
    int addrlen = sizeof(address);  // Length of address structure
    char ip[64];                    // IP for hostname resolution
    struct addrinfo hints;          // Hints for hostname resolution
    struct addrinfo *info;          // Info for hostname resolution
    pack_t pack;                    // The packet sctructure for communication
    //////////////////////////////////

    // Open Socket
    printf("Opening SLAVE socket...\n");
    ERR_EXIT(sock = socket(AF_INET, SOCK_STREAM, 0)); // IPv4 / TCP
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    ERR_EXIT(inet_pton(AF_INET, host, &address.sin_addr));
    printf("Connecting to '%s' in port %d...\n", host, port);
    ERR_EXIT(connect(sock, &address, addrlen));
    printf("Connected!\n");

    bzero(&pack, sizeof(pack));
    pack.type = CALC_REQUEST;
    printf("Requesting calculus to master...\n");
    pack_send(sock, &pack);
    pack_recv(sock, &pack);
    switch (pack.type) {
        case CALC_RESPONSE:
            printf("Calculating from %g to %g, with dx = %g...\n", pack.min, pack.max, pack.dx_res);
            err = calc_integ(func, pack.min, pack.max, pack.dx_res, &(pack.dx_res));
            if (err < 0) {
                pack.type = CALC_FAIL;
                pack.dx_res = UNDEF;
                printf("Calculus failed: %s\n", strerror(errno));
            } else {
                pack.type = CALC_RESULT;
                printf("Result: %g\n", pack.dx_res);
            }
            printf("Sending to master...\n");
            pack_send(sock, &pack);
            break;
        case CALC_NONEED:
            printf("Master doesn't need me. Good bye!\n");
            return;
        default:
            printf("Master is crazy. I can't understand it :S\n");
    }

    close(sock);
    printf("My service is done! Goodbye!\n");
}

int main(int argc, char const *argv[]) {
    char host[64];          // the master host (default "localhost")
    strcpy(host, "localhost");
    int i;                  // the loop iterator
    int err;                // the error checker
    int port = 8989;        // the port master listen to (default 8989)
    int reiter = 0;         // slave's reiteration flag
    int type = UNDEF;       // the type of this process (MASTER | SLAVE)
    int number = 1;         // the number of slaves master expects (default 1)
    double step = 0.0001;   // the step size for the integration
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
        slave(port, host, reiter);
    }
    
    return 0;
}
