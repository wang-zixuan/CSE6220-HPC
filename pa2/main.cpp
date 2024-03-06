#include "mpi.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>


int HPC_Alltoall_H(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    
}

int main(int argc, char* argv[]) {
    int rank, size;
    int n;
    char alg_name;
    std::string source_file_name, target_file_name;

    // MPI initialziation
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = std::stod(argv[4]);
    // a: HPC_Alltoall_A; h: HPC_Alltoall_H, m: MPI_Alltoall
    alg_name = argv[3][0];

    if (rank == 0) {
        source_file_name = argv[1];
        target_file_name = argv[2];

        // read source file
        std::ifstream file(source_file_name);
        std::string str;
        std::vector<std::vector<int>> arr;
        // Process txt file
        while (std::getline(file, str)) {
            std::stringstream ss(str);
            std::vector<int> row;
            int tmp;

            while (ss >> tmp) {
                row.push_back(tmp);
            }

            arr.push_back(row);
        }

        file.close();

        // scatter
        
    }


    MPI_Finalize();
    return 0;
}
