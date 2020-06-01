#ifndef _DS_INTEG_MASTER_H
#define _DS_INTEG_MASTER_H

// Master main routine
// 
// port: port to listen as a server
// slaves: number of expected slaves
// dx: discretization interval for trapeze method
void master(int port, int slaves, double dx);

#endif
