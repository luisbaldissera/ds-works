#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define SLAVE_MAX   10
#define MASTER      1
#define SLAVE       2

#define X_IDX       0
#define XDX_IDX     1

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

#define UNDEF       ~0

typedef struct {
    double x;
    double fx;
    double (*func)(double);
} func_cache_t;

typedef struct {
    int type;
    double min;
    double max;
    double dx_res;
} pack_t;

extern int errno;

void master(int port, int slaves, double dx);
void slave(int port, char const *host, int reiter);

int calc_integ(double (*f)(double), double min, double max, double dx, double *result);
void print_usage();
void bad_usage();

void print_usage() {
    fprintf(stderr, "USAGE\n\n");
    fprintf(stderr, "\tinteg master [-n NUMBER] [-s VALUE] [-p PORT]\n");
    fprintf(stderr, "\tinteg slave [-p PORT] [-m HOST] [-r]\n");
    fprintf(stderr, "\tinteg -h\n");
    fprintf(stderr, "\nOPTIONS\n\n");
    fprintf(stderr, "\t-n|--number NUMBER\n");
    fprintf(stderr, "\tThe number of expected slaves.\n");
    fprintf(stderr, "\tdefault: 1\n\n");
    fprintf(stderr, "\t-s|--step VALUE\n");
    fprintf(stderr, "\tThe step size for trapeze calculus.\n");
    fprintf(stderr, "\tdefault: 0.0001\n\n");
    fprintf(stderr, "\t-p|--port PORT\n");
    fprintf(stderr, "\tThe port which master listens to.\n");
    fprintf(stderr, "\tdefault: 8989\n\n");
    fprintf(stderr, "\t-m|--master HOST\n");
    fprintf(stderr, "\tThe master's host.\n");
    fprintf(stderr, "\tdefault: localhost\n\n");
    fprintf(stderr, "\t-r|--reiter\n");
    fprintf(stderr, "\tWhen this flag is present, slave makes another\n");
    fprintf(stderr, "\trequest for master after finished.\n\n");
    fprintf(stderr, "\t-h|--help\n");
    fprintf(stderr, "\tShow this message.\n\n");
    exit(0);
}

void bad_usage() {
        fprintf(stderr, "Bad usage.\nSee 'integ --help'.\n");
        exit(1);
}

double func (double x) {
    return sqrt(10000.0 - x*x);
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
    int srv;                        // Server socket
    int sock;                       // Response socket
    int err;                        // Error auxiliar
    int i;                          // Loop iterator
    double sum;                     // Slaves' instegrate sum
    int deliv;                      // Number of delivered calculus
    int chunk = slaves;             // Number of chunks of calculus
    double size = 100.0 / slaves;   // How much each slave will calc
    double start = 0.0;             // The interval begin
    pack_t pack;                    // Package of the communication
    struct sockaddr_in address;     // The server socket address
    int addrlen = sizeof(address);
    // Open Server socket
    printf("Opening MASTER socket...\n");
    srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(errno);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    // Attach server socket to port
    printf("Attaching socket to port %d...\n", port);
    err = bind(srv, &address, addrlen);
    if (err < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(errno);
    }
    // Start listening
    err = listen(srv, 3);
    if (err < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(errno);
    }
    printf("Listening on port %d...\n", port);
    deliv = 0;
    chunk = slaves;
    // Starts server routine
    while (deliv > 0 || chunk > 0) {
        sock = accept(srv, &address, &addrlen);
        if (sock < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(errno);
        }
        err = read(sock, &pack, sizeof(pack));
        if (err < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(errno);
        }

        switch (pack.type) {
            case CALC_REQUEST:
                printf("Slave requested for calculus.\n");
                if (chunk == 0) { // All calculus was already delivered
                    pack.type = CALC_NONEED;
                    printf("No need for more calculus.\n");
                } else {
                    pack.type = CALC_RESPONSE;
                    pack.min = start;
                    start += size;
                    pack.max = start;
                    pack.dx_res = dx;
                    chunk--;
                    deliv++;
                    printf("Calculates from %g to %g with dx = %g\n", pack.min, pack.max, pack.dx_res);
                }
                break;
            case CALC_RESULT:
                printf("Slave ended calculus.\n");
                printf("From %g to %g. Result: %g\n", pack.min, pack.max, pack.dx_res);
                sum += pack.dx_res;
                deliv--;
                continue;
            case CALC_FAIL:
                printf("Slave's calculus failed.\n");
                chunk++;
                deliv--;
                // see what failed
                pack.type = CALC_NONEED;
                break;
            default:
                fprintf(stderr, "Error: Package error\n");
                pack.type = CALC_NONEED;
        }

        err = send(sock, &pack, sizeof(pack), 0);
        if (err < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(errno);
        }
    }
    printf("Result: %g\n", sum);
}

void slave(int port, char const *host, int reiter) {
    char buffer[1024];  // {DEPRECIATED}
    int sock;                       // Socket to connect to master
    int err;                        // Error auxiliar
    struct sockaddr_in address;     // Server address
    int addrlen = sizeof(address);
    pack_t pack;
    printf("Opening SLAVE socket...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(errno);
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    err = inet_pton(AF_INET, host, &address.sin_addr);
    if (err < 0) {
        fprintf(stderr, "Error: Invalid Address: %s\n", strerror(errno));
        exit(errno);
    }
    printf("Connecting to '%s' in port %d...\n", host, port);
    err = connect(sock, &address, addrlen);
    if (err < 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(errno);
    }
    printf("Connected!\n");
    do {
        pack.type = CALC_REQUEST;
        printf("Requesting calculus to master...\n");
        err = send(sock, &pack, sizeof(pack), 0);
        if (err < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(errno);
        }
        err = read(sock, &pack, sizeof(pack));
        if (err < 0) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            exit(errno);
        }
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
                err = send(sock, &pack, sizeof(pack), 0);
                if (err < 0) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                    exit(errno);
                }
                break;
            case CALC_NONEED:
                printf("Master doesn't need me. Good bye!\n");
                return;
            default:
                printf("Master is crazy. I can't understand it :S\n");
        }
        if (reiter) {
            printf("Let's do it again.\n");
        }

    } while (reiter);
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
    double step = UNDEF;    // the step size for the integration
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
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reiter") == 0) {
            if (type == MASTER) {
                fprintf(stderr, "Warning: useless option '%s' for type master\n", argv[i]);
            }
            reiter = 1;
            i++;
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
