#include "integrate.h"

double integrate(double (*f)(double), double min, double max, double dx) {
    double cache;                   // Cache to avoid repeated caclulus
    double trapeze;                 // Trapeze area
    double const fac = dx / 2.0;    // Trapeze factor multiplication
    double x;                       // The X axe iterator
    double sum;                     // The sum of integration
    sum = 0;
    cache = f(min);
    x = min + dx;
    while (x <= max) {
        trapeze = cache;
        cache = f(x);
        trapeze = (trapeze + cache) * fac; // Calculates trapeze area
        sum += trapeze;
        x += dx;
    }
    return sum;
}
