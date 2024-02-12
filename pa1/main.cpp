#include "mpi.h"
#include <iostream>

#define MASTER 0

int main(int argc, char* argv[]) {
    int rank, size;
    long long n, local_count = 0, total_count;
    double pi_estimate = 0.0, start_time, end_time, x, y;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // number of points
    if (rank == MASTER) {
        n = (long long) (std::stod(argv[1]));
        start_time = MPI_Wtime();
    }

    MPI_Bcast(&n, 1, MPI_LONG_LONG_INT, MASTER, MPI_COMM_WORLD);
    
    srand(time(NULL) + rank);

    for (long long i = rank + 1; i <= n; i += size) {
        x = (double) rand() / RAND_MAX;
        y = (double) rand() / RAND_MAX;
        if (x * x + y * y <= 1) {
            local_count++;
        }
    }

    MPI_Reduce(&local_count, &total_count, 1, MPI_LONG_LONG_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        pi_estimate = (4.0 * total_count) / (double) n;
        end_time = MPI_Wtime();
        printf("%.12f, %f\n", pi_estimate, end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
