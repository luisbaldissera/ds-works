#ifndef _DS_INTEG_COMMONS_H
#define _DS_INTEG_COMMONS_H

/////////////////////////////////////////////////////////////////////////////
// DEFAULTS /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#define DEFAULT_PORT    8989
#define DEFAULT_MASTER  "localhost"
#define DEFAULT_STEP    0.0001
#define DEFAULT_SLAVES  1

/////////////////////////////////////////////////////////////////////////////
// ERRORS ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Macros and includes to handle errors with 'errno'
//////////////////////
#include <errno.h>  // errno
extern int errno;   //
#include <string.h> // strerror
//////////////////////
// ERR_EXIT receives a command that, at the end, returns an integer.
// If the execution of this command returns a negative number, then
// the program exits, and information about the error stored in 'errno'
// is printed.
#define ERR_EXIT(cmd) do{if((int)(cmd)<(0)){fprintf(stderr,"Error (%s:%d): %s\n",__FILE__,__LINE__,strerror(errno));exit(EXIT_FAILURE);}}while(0)
/////////////////////
// THROW is similar to error exit, but it receives the 'errno' code instead,
// exits the calling thread, and set 'errno' accordingly
#define THROW(err) ERR_EXIT(-(errno = (err)))

/////////////////////////////////////////////////////////////////////////////
// PROTOCOL /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// To comunicate between master and slaves, it is defined a protocol.
// At first, master is used as a server waiting for slaves connect
// as clients. When a slave connects to master, it first request to
// it for an interval for calculus using a CALC_REQUEST package. Then
// master can answer this requeste with a CALC_RESPONSE package,
// containing an interval and discretization level; or with a
// CALC_NONEED package, when it has already delivered all the intervals
// for other slaves and don't need slaves anymore. Finally, after
// making the calculus, the slave is going to send a CALC_RESULT
// package with the result of its calculus; or a CALC_FAIL if it
// couldn't succed calculation.
#define CALC_REQUEST    1
#define CALC_NONEED     2
#define CALC_RESPONSE   3
#define CALC_RESULT     4
#define CALC_FAIL       5
// All the package information is stored in following structure
typedef struct {
    int type;
    double min;
    double max;
    double dx_res;
} pack_t;
// Sends a package over a socket
void pack_send(int sock, pack_t *src);
// Receives a package from a socket
void pack_recv(int sock, pack_t *dest);

///////////////////////////////////////////////////////////////////////
// A special printf that can be disllowed by set_print. By default it
// is allowed. The usage is exactaly same as printf. To disallow, use
// set_print(0)
int print(char const *fmt, ...);
void set_print(int set);

//////////////////////////////////////////////////////////////////////
// TIMING ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// tik and tok are defined to measure time. When use tik, timer is
// reset, and when use tok, the number of milliseconds past from last 
// tik is returned.
void tic();
double toc();

#endif
