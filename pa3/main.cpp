#include "mpi.h"
#include <string>
#include <tuple>
#include <vector>
#include <random>
#include <chrono>

struct SparseMatrixEntry {
    uint64_t row;
    uint64_t col;
    uint64_t value;
};

MPI_Datatype MPI_SPARSE_ENTRY;

std::vector<SparseMatrixEntry> generateSparseMatrix(uint64_t rows, uint64_t n, double sparsity, int rank) {
    // generate values for n/p rows and n cols
    auto seed = std::chrono::system_clock::now().time_since_epoch().count() + rank;
    std::mt19937 rngSparsity(static_cast<unsigned int>(seed));
    std::uniform_real_distribution<double> distSparsity(0.0, 1.0);

    auto value_seed = std::chrono::system_clock::now().time_since_epoch().count() / (rank + 1);
    std::mt19937_64 rngValue(static_cast<unsigned int>(value_seed));
    std::uniform_int_distribution<uint64_t> distValue(0, UINT64_MAX);

    std::vector<SparseMatrixEntry> sparseMatrix;

    for (uint64_t i = 0; i < rows; i++) {
        for (uint64_t j = 0; j < n; j++) {
            if (distSparsity(rngSparsity) < sparsity) {
                uint64_t val = distValue(rngValue);
                SparseMatrixEntry s;
                s.row = i;
                s.col = j;
                s.value = val;
                sparseMatrix.push_back(s);
            }
        }
    }

    return sparseMatrix;
}

int main(int argc, char* argv[]) {
    int rank, size;

    uint64_t n;
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

    // create MPI MPI_SPARSE_ENTRY type
    const int nitems = 3;
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_UINT64_T, MPI_UINT64_T, MPI_UINT64_T};

    MPI_Aint offsets[3];
    offsets[0] = offsetof(SparseMatrixEntry, row);
    offsets[1] = offsetof(SparseMatrixEntry, col);
    offsets[2] = offsetof(SparseMatrixEntry, value);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_SPARSE_ENTRY);
    MPI_Type_commit(&MPI_SPARSE_ENTRY);

    // generate sparse matrices A and B
    std::vector<SparseMatrixEntry> sparseMatrixA = generateSparseMatrix(n / size, n, sparsity_parameter, rank);
    std::vector<SparseMatrixEntry> sparseMatrixB = generateSparseMatrix(n / size, n, sparsity_parameter, rank);

    // transpose matrix B

    // step 1: prepare send counts and send buffer
    std::vector<int> send_counts(size, 0);
    std::vector<std::vector<SparseMatrixEntry>> sendbuffer_2d(size);
    std::vector<SparseMatrixEntry> send_buffer;

    for (const auto& entry : sparseMatrixB) {
        int col = entry.col;
        int target_proc = col / (n / size);
        sendbuffer_2d[target_proc].emplace_back(entry);
    }

    for (int i = 0; i < size; i++) {
        send_counts[i] = sendbuffer_2d[i].size();
        send_buffer.insert(send_buffer.end(), sendbuffer_2d[i].begin(), sendbuffer_2d[i].end());
    }

    // step 2: communicate the send counters
    std::vector<int> recv_counts(size);
    MPI_Alltoall(send_counts.data(), 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    // step 3: compute displacements
    std::vector<int> sdispls(size, 0), rdispls(size, 0);
    std::partial_sum(send_counts.begin(), send_counts.end() - 1, sdispls.begin() + 1);
    std::partial_sum(recv_counts.begin(), recv_counts.end() - 1, rdispls.begin() + 1);

    // step 4: perform transpose with MPI_Alltoallv
    std::vector<SparseMatrixEntry> recv_buffer(std::accumulate(recv_counts.begin(), recv_counts.end(), 0));
    MPI_Alltoallv(send_buffer.data(), send_counts.data(), sdispls.data(), MPI_SPARSE_ENTRY, 
                  recv_buffer.data(), recv_counts.data(), rdispls.data(), MPI_SPARSE_ENTRY, MPI_COMM_WORLD);


    MPI_Finalize();
    return 0;
}
