#include "mpi.h"
#include <string>

int main(int argc, char* argv[]) {
    int rank, size;

    int n;
    double sparsity_parameter;
    int print_flag;
    std::string output_file_name;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = std::stoi(argv[1]);
    sparsity_parameter = std::stod(argv[2]);
    print_flag = std::stoi(argv[3]);
    output_file_name = argv[4];

    MPI_Finalize();
    return 0;
}