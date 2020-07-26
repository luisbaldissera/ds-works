#ifndef _DS_INTEG_COMMONS_H
#define _DS_INTEG_COMMONS_H

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

///////////////////////////////////////////////////////////////////////
// A special printf that can be disllowed by set_print. By default it
// is allowed. The usage is exactaly same as printf. To disallow, use
// set_print(0)
int print(char const *fmt, ...);
void set_print(int set);

//////////////////////////////////////////////////////////////////////
// TIMING ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// tic and toc are defined to measure time. When use tic, timer is
// reset, and when use toc, the number of milliseconds past from last 
// tic calling is returned.
void tic();
double toc();

//////////////////////////////////////////////////////////////////////
// MPI DEFINITIONS ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#define TAG_RESULT      1
#define DEFAULT_NP      2

#endif
