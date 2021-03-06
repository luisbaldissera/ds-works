#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

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

// f(x) = sqrt{100^2 - x^2}
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
    /////////////////////////////////////////////////
    // Args verify
    /////////////////////////////////////////////////
    if (argc < 2) {
        bad_usage();
    }
    // help option
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage();
    // master type
    } else if (strcmp(argv[1], "master") == 0) {
        type = MASTER;
    // slave type
    } else if (strcmp(argv[1], "slave") == 0) {
        type = SLAVE;
    } else {
        fprintf(stderr, "Error: Neighter 'master' or 'slave'.\n");
        bad_usage();
    }
    i = 2;
    // By default, printing is enabled
    set_print(1);
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
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            set_print(0);
            i++;
        } else {
            fprintf(stderr, "Error: invalid option '%s'.\n", argv[i]);
            bad_usage();
        }
    }
    // END arguments verification
    switch (type) {
        case MASTER:
            master(port, number, step);   // master main routine
            break;
        case SLAVE:
            slave_set_function(func);     // set function to sqrt(100^2 - x^2)
            slave(host, port);      	  // slave main routine
            break;
        default:
            fprintf(stderr, "Error: undefinied master/slave.\n");
    }
    return 0;
}
