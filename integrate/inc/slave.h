#ifndef _DS_INTEG_SLAVE_H
#define _DS_INTEG_SLAVE_H

void slave_set_function(double (*f)(double));
void slave(char const *host, int port);

#endif
