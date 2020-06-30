#include "integrate.h"

double integrate(double (*f)(double), double lmin, double lmax, double dx) {
    double cache;                   // Cache to avoid repeated caclulus
    double trapeze;                 // Trapeze area
    double const fac = dx / 2.0;    // Trapeze factor multiplication
    double x;                       // The X axe iterator
    double sum;                     // The sum of integration
    sum = 0;
    cache = f(lmin);
    x = lmin + dx;
    while (x <= lmax) {
        trapeze = cache;
        cache = f(x);
        trapeze = (trapeze + cache) * fac; // Calculates trapeze area
        sum += trapeze;
        x += dx;
    }
    return sum;
}

double partinteg(double (*f)(double), double gmin, double gmax, double dx, int id, int total) {
    double lmin, lmax, len;
    len = (gmax - gmin) / total;
    lmin = id * len;
    lmax = lmin + len;
    if (lmax > gmax) lmax = gmax;
    return integrate(f, lmin, lmax, dx);
}
