#ifndef _DS_INTEGRATE_H
#define _DS_INTEGRATE_H

// Make the integration of the function 'f':(double)->(double) in the interval
// ['min','max'] with trapeze method with discretization interval 'dx'
double integrate(double (*f)(double), double min, double max, double dx);

#endif
