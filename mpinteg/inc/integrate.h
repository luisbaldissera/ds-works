#ifndef _DS_INTEGRATE_H
#define _DS_INTEGRATE_H

// Make the integration of the function 'f':(double)->(double) in the interval
// ['lmin','lmax'] with trapeze method with discretization interval 'dx'
//
// where 'lmin' is Local Minimum e 'lmax' is Local Maximum
double integrate(double (*f)(double), double lmin, double lmax, double dx);

// Make the integration of the function 'f':(double)->(double) in a piece of the
// interval ['gmin','gmax'] calling the integrate trapeze method. This subinterval
// is calculates proportionally, given numbers of 'id' and 'total'.
//
// where 'gmin' is Global Minimum and 'gmax' is Global Maximum
double partinteg(double (*f)(double), double gmin, double gmax, double dx, int id, int total);

#endif
