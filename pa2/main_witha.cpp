#include "mpi.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <cstring>

std::vector<int> pre_transpose(std::vector<int>& matrix, int block_size, int n) {
    int row_size = block_size / n, col_size = n;
    std::vector<int> transposed;
    for (int j = 0; j < col_size; j++) {
        for (int i = 0; i < row_size; i++) {
            transposed.push_back(matrix[i * col_size + j]);
        }
    }

    return transposed;
}

std::vector<int> post_transpose(std::vector<int>& matrix, int n, int size) {
    int col_size = n / size;
    std::vector<int> res;
    for (int i = 0; i < col_size; i++) {
        for (int j = 0; j < size; j++) {
            res.insert(res.end(), matrix.begin() + j * col_size * col_size + col_size * i, matrix.begin() + j * col_size * col_size + col_size * (i + 1));
        }
    }

    return res;
}

int HPC_Alltoall_A(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    int size, rank;
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    
    const int* sendbuf_int = static_cast<const int*>(sendbuf);
    int* recvbuf_int = static_cast<int*>(recvbuf);

    MPI_Status status;
    MPI_Request send_request, recv_request;

    std::memcpy(recvbuf_int + rank * sendcount, sendbuf_int + rank * sendcount, sendcount * sizeof(int));

    for (int j = 1; j < size; ++j) {
        int send_to = (rank + j) % size;
        int recv_from = (rank - j + size) % size;
        
        MPI_Irecv(recvbuf_int + recv_from * recvcount, recvcount, recvtype, recv_from, 0, comm, &recv_request);
        
        MPI_Isend(sendbuf_int + rank * sendcount, sendcount, sendtype, send_to, 0, comm, &send_request);
        
        MPI_Wait(&send_request, &status);
        MPI_Wait(&recv_request, &status);
    }

    return MPI_SUCCESS;
}






int HPC_Alltoall_H(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm) {
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int buffer_size = sendcount * size;
    int d = log2(size);

    int flip = 1 << (d - 1);
    int* sendbuf_int = new int[buffer_size];
    // cast to int* since we can't retreive data in void* datatype
    const int* sendbuf_int_cast = static_cast<const int*>(sendbuf);
    // copy data from void* sendbuf to int* sendbuf
    for (int i = 0; i < buffer_size; i++) {
        sendbuf_int[i] = sendbuf_int_cast[i];
    }
    
    int* recvbuf_int = static_cast<int*>(recvbuf);

    // the length of send buf is always buffer_size / 2 in every round
    std::vector<int> local_send_buf(buffer_size / 2);
    for (int j = d - 1; j >= 0; j--) {
        // compute target rank by flipping the j-th bit
        int target_rank = rank ^ flip;
        // For example, if we have 8 processors, 
        // in the first round, rank 0 will send last 4 blocks of sendbuf to rank 4, rank 4 will send first 4 blocks to rank 0
        // in the second round, rank 0 will send block 2, 3, 6, 7 to rank 2
        // in the third round, rank 0 will send block 1, 3, 5, 7 to rank 1
        int comm_size = buffer_size / (1 << (d - j));
        int transfer_time = 1 << (d - 1 - j);
        int start_idx = 0;
        for (int i = 0; i < transfer_time; i++) {
            // get the (start index, start index + comm_size) elements of the sendbuf which needs to be sent to the target rank
            int offset = (target_rank & flip) * sendcount + buffer_size / transfer_time * i;
            for (int k = start_idx; k < start_idx + comm_size; k++) {
                local_send_buf[k] = sendbuf_int[offset + k - start_idx];
            }
            start_idx += comm_size;
        }

        // send and receive between current rank and target rank
        MPI_Send(local_send_buf.data(), buffer_size / 2, sendtype, target_rank, 0, comm);
        MPI_Status status;
        MPI_Recv(local_send_buf.data(), buffer_size / 2, sendtype, target_rank, 0, comm, &status);
        start_idx = 0;
        // put data from local send buf to send buffer
        for (int i = 0; i < transfer_time; i++) {
            int offset = (target_rank & flip) * sendcount + buffer_size / transfer_time * i;
            for (int k = 0; k < comm_size; k++) {
                sendbuf_int[offset + k] = local_send_buf[start_idx + k];
            }
            start_idx += comm_size;
        }

        // next round
        flip = flip >> 1;
    }

    // copy data from send buf to recv buf
    std::copy(sendbuf_int, sendbuf_int + buffer_size, recvbuf_int);

    return 0;
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
    // command line argument input: 
    //         a: HPC_Alltoall_A; 
    //         h: HPC_Alltoall_H, 
    //         m: MPI_Alltoall
    alg_name = argv[3][0];

    int block_size = n * n / size;

    // arr is to store matrix
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

    // scatter array data to all processors
    MPI_Scatter(arr.data(), block_size, MPI_INT, recv_scatter.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

    // timer begins here
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    std::vector<int> recv_scatter_transposed = pre_transpose(recv_scatter, block_size, n);
    std::vector<int> recv_alltoall(block_size);

    // all to all communication
    if (alg_name == 'a') {
        // implement arbitrary all-to-all here
        HPC_Alltoall_A(recv_scatter_transposed.data(), block_size / size, MPI_INT, recv_alltoall.data(), block_size / size, MPI_INT, MPI_COMM_WORLD);
    } else if (alg_name == 'h') {
        HPC_Alltoall_H(recv_scatter_transposed.data(), block_size / size, MPI_INT, recv_alltoall.data(), block_size / size, MPI_INT, MPI_COMM_WORLD);
    } else {
        MPI_Alltoall(recv_scatter_transposed.data(), block_size / size, MPI_INT, recv_alltoall.data(), block_size / size, MPI_INT, MPI_COMM_WORLD);
    }

    // timer ends here
    if (rank == 0) {
        end_time = MPI_Wtime();
    }

    std::vector<int> recv_alltoall_transposed = post_transpose(recv_alltoall, n, size);

    // gather data from all processors to rank 0 processor
    MPI_Gather(recv_alltoall_transposed.data(), block_size, MPI_INT, arr.data(), block_size, MPI_INT, 0, MPI_COMM_WORLD);

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
