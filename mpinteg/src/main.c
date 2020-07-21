#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strcmp
#include <math.h>   // sqrt
#include <mpi.h>

#include "integrate.h"
#include "commons.h"

// f(x) = sqrt{100^2 - x^2}
double func (double x) {
    return sqrt(10000.0 - x*x);
}

int main(int argc, char *argv[]) {
    int id;         // rank of this process
    int total;      // total number of process
    int rc;         // auxiliar variable to threat errors
    double sum;     // sum of this process
    double result;  // final result
    double dx;      // discretization interval
    double ms;      // execution time, in miliseconds
    int i;          // loop iteration
    set_print(1);
    if (argc < 2) {
        fprintf(stderr, "Error: No dx given.\n");
        return EXIT_FAILURE;
    }
    rc = sscanf(argv[1], "%lf", &dx);
    if (rc < 1) {
        fprintf(stderr, "Error: Cannot convert \'%s\' to float.\n", argv[1]);
        return EXIT_FAILURE;
    }
    i = 2;
    while (i < argc) {
        if (strcmp(argv[i], "-t") == 0) {
            set_print(0);
        }
        i++;
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &total);
    if (id == 0)
        tic();
    sum = partinteg(func, 0.0, 100.0, dx, id, total);
    MPI_Reduce(&sum, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (id == 0) {
        ms = toc();
        print("Result: %g\n", result);
        printf("%.3lf ms\n", ms);
    }
    MPI_Finalize();
    return 0;
}
