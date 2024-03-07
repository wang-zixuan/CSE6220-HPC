#include "mpi.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

void local_transpose(std::vector<int>& matrix, int block_size, int n) {
    int row_size = n, col_size = block_size / n;
    for (int i = 0; i < row_size; i++) {
        for (int j = i + 1; j < col_size; j++) {
            int tmp = matrix[i * col_size + j];
            matrix[i * col_size + j] = matrix[j * col_size + i];
            matrix[j * col_size + i] = tmp;
        }
    }
}

int HPC_Alltoall_A(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {

}


int HPC_Alltoall_H(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    
}

int main(int argc, char* argv[]) {
    int rank, size;
    int n;
    char alg_name;
    std::string source_file_name, target_file_name;
    double start_time, end_time;

    // MPI initialziation
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = std::stod(argv[4]);
    // a: HPC_Alltoall_A; h: HPC_Alltoall_H, m: MPI_Alltoall
    alg_name = argv[3][0];

    int block_size = n * n / size;

    std::vector<int> arr;

    if (rank == 0) {
        source_file_name = argv[1];
        target_file_name = argv[2];

        // read source file
        std::ifstream file(source_file_name);
        std::string str;
        // Process txt file
        while (std::getline(file, str)) {
            std::stringstream ss(str);
            int tmp;

            while (ss >> tmp) {
                arr.push_back(tmp);
            }
        }

        file.close();
    }

    // receive buffer for scatter
    std::vector<int> recv_scatter(block_size);

    // scatter
    MPI_Scatter(arr.data(), block_size, MPI_INT, recv_scatter.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // timer begins here
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    local_transpose(recv_scatter, block_size, n);
    std::vector<int> recv_alltoall(block_size);

    // all to all communication
    if (alg_name == 'a') {
        // implement arbitrary all-to-all here
        HPC_Alltoall_A(recv_scatter.data(), block_size, MPI_INT, recv_alltoall.data(), block_size, MPI_INT, MPI_COMM_WORLD);
    } else if (alg_name == 'h') {
        HPC_Alltoall_H(recv_scatter.data(), block_size, MPI_INT, recv_alltoall.data(), block_size, MPI_INT, MPI_COMM_WORLD);
    } else {
        MPI_Alltoall(recv_scatter.data(), block_size, MPI_INT, recv_alltoall.data(), block_size, MPI_INT, MPI_COMM_WORLD);
    }

    // timer ends here
    if (rank == 0) {
        end_time = MPI_Wtime();
    }

    // gather
    MPI_Gather(recv_alltoall.data(), block_size, MPI_INT, arr.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // store result into txt file
    if (rank == 0) {
        std::ofstream outputFile(target_file_name);
        for (int i = 0; i < arr.size(); i++) {
            outputFile << arr[i];
            if ((i + 1) % n != 0) {
                outputFile << " ";
            } else {
                outputFile << std::endl;
            }
        }

        outputFile.close();
        // print time
        printf("%.6f\n", end_time - start_time);
    }

    MPI_Finalize();
    return 0;
}
