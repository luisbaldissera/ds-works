#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "integrate.h"
#include "commons.h"

// f(x) = sqrt{100^2 - x^2}
double func (double x) {
    return sqrt(10000.0 - x*x);
}

int main(int argc, char *argv[]) {
    int id;
    int total;
    int rc;
    double sum;
    double neigh;
    double dx = 0.0001;
    if (argc < 2) {
        fprintf(stderr, "Error: No dx given.\n");
        return EXIT_FAILURE;
    }
    rc = sscanf(argv[1], "%lf", &dx);
    if (rc < 1) {
        fprintf(stderr, "Error: Cannot convert \'%s\' to float.\n", argv[1]);
        return EXIT_FAILURE;
    }
    set_print(1);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &total);
    sum = partinteg(func, 0.0, 100.0, dx, id, total);
    if (id + 1 != total) {
        MPI_Recv(&neigh, 1, MPI_DOUBLE, id + 1, TAG_RESULT, MPI_COMM_WORLD, NULL);
        sum += neigh;
    }
    if (id != 0) {
        MPI_Send(&sum, 1, MPI_DOUBLE, id - 1, TAG_RESULT, MPI_COMM_WORLD);
    } else {
        print("Result: %g\n", sum);
    }
    MPI_Finalize();
    return 0;
}
