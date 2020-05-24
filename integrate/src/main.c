#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cat.h"
#include "commons.h"
#include "slave.h"
#include "master.h"

#define MASTER      1
#define SLAVE       2

#define HELP_PATH "assets/help.txt"

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
    
    switch (type) {
        case MASTER:
            master(port, number, step);
            break;
        case SLAVE:
            slave_set_function(func);
            slave(host, port);
            break;
        default:
            fprintf(stderr, "Error: undefinied master/slave.\n");

    }
    
    return 0;
}
