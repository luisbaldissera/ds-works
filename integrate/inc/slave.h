#ifndef _DS_INTEG_SLAVE_H
#define _DS_INTEG_SLAVE_H


// Sets the function that will be integrated by slave
void slave_set_function(double (*f)(double));

// The slave main routine
//
// host: hostname or ip (in dot form "*.*.*.*") which is master
// port: port which master is listening to
void slave(char const *host, int port);

#endif
