#include "mpi.h"

#define MASTER 0

void hypercubic_all_to_all() {
    
}

int main(int argc, char* argv[]) {
    int rank, size;

    // MPI initialziation
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Finalize();
    return 0;
}
