#include "mpi.h"
#include <string>
#include <tuple>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>

struct SparseMatrixEntry {
    uint64_t row;
    uint64_t col;
    uint64_t value;
};

MPI_Datatype MPI_SPARSE_ENTRY;

std::vector<SparseMatrixEntry> generateSparseMatrix(uint64_t row_start, uint64_t row_end, uint64_t n, double sparsity, int rank) {
    // generate values for n/p rows and n cols
    // seed for sparsity
    auto seed = std::chrono::system_clock::now().time_since_epoch().count() + rank;
    std::mt19937 rngSparsity(static_cast<unsigned int>(seed));
    std::uniform_real_distribution<double> distSparsity(0.0, 1.0);

    // for generating value, use another seed
    auto value_seed = std::chrono::system_clock::now().time_since_epoch().count() / (rank + 1);
    std::mt19937_64 rngValue(static_cast<unsigned int>(value_seed));
    std::uniform_int_distribution<uint64_t> distValue(0, UINT16_MAX);

    std::vector<SparseMatrixEntry> sparseMatrix;

    for (uint64_t i = row_start; i < row_end; i++) {
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

    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = std::stoull(argv[1]);
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
    uint64_t row_start = n / size * rank;
    uint64_t row_end = n / size * (rank + 1);
    
    std::vector<SparseMatrixEntry> sparseMatrixA = generateSparseMatrix(row_start, row_end, n, sparsity_parameter, rank);
    std::vector<SparseMatrixEntry> sparseMatrixB = generateSparseMatrix(row_start, row_end, n, sparsity_parameter, rank);

    // initialize C with size: [n / size, n]
    uint64_t* local_c = (uint64_t*)calloc(n * n / size, sizeof(uint64_t));
    uint64_t* flattened_local_a = (uint64_t*)calloc(n * n / size, sizeof(uint64_t));
    uint64_t* flattened_local_b = (uint64_t*)calloc(n * n / size, sizeof(uint64_t));

    for (const auto& entry : sparseMatrixA) {
        uint64_t row = entry.row;
        uint64_t col = entry.col;
        flattened_local_a[(row - row_start) * n + col] = entry.value;
    }

    for (const auto& entry : sparseMatrixB) {
        uint64_t row = entry.row;
        uint64_t col = entry.col;
        flattened_local_b[(row - row_start) * n + col] = entry.value;
    }

    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    std::unordered_map<uint64_t, std::vector<SparseMatrixEntry>> groupedA;

    for (const auto& entry : sparseMatrixA) {
        groupedA[entry.col].push_back(entry);
    }

    // step 1: prepare send counts and send buffer
    std::vector<int> send_counts(size, 0);
    std::vector<std::vector<SparseMatrixEntry>> sendbuffer_2d(size);
    std::vector<SparseMatrixEntry> send_buffer;

    for (const auto& entry : sparseMatrixB) {
        uint64_t col = entry.col;
        int target_proc = col / (n / size);
        sendbuffer_2d[target_proc].emplace_back(entry);
    }

    // flatten the 2D send buffer to 1D
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
    // recv_buffer stores non-zero entries of n/p columns of B locally
    std::vector<SparseMatrixEntry> recv_buffer(std::accumulate(recv_counts.begin(), recv_counts.end(), 0));
    MPI_Alltoallv(send_buffer.data(), send_counts.data(), sdispls.data(), MPI_SPARSE_ENTRY, 
                  recv_buffer.data(), recv_counts.data(), rdispls.data(), MPI_SPARSE_ENTRY, MPI_COMM_WORLD);

    // compute local data first
    for (const auto& entryB : recv_buffer) {
        auto it = groupedA.find(entryB.row);
        if (it != groupedA.end()) {
            for (const auto& entryA : it->second) {
                local_c[(entryA.row - row_start) * n + entryB.col] += entryA.value * entryB.value;
            }
        }
    }

    // ring topology, rotate B (size - 1) times
    // recv_buffer becomes send_buffer now
    int source = (rank - 1 + size) % size;
    int dest = (rank + 1) % size;

    std::vector<SparseMatrixEntry> send_buffer_ring_topology(recv_buffer.begin(), recv_buffer.end());
    for (int i = 0; i < size - 1; i++) {
        int send_size = send_buffer_ring_topology.size();
        int recv_size;

        // communicate count
        MPI_Sendrecv(&send_size, 1, MPI_INT, dest, 0, &recv_size, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::vector<SparseMatrixEntry> recv_buffer_ring_topology(recv_size);

        // communicate real data
        MPI_Sendrecv(send_buffer_ring_topology.data(), send_size, MPI_SPARSE_ENTRY, dest, 0, recv_buffer_ring_topology.data(), recv_size, MPI_SPARSE_ENTRY, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (const auto& entryB : recv_buffer_ring_topology) {
            auto it = groupedA.find(entryB.row);
            if (it != groupedA.end()) {
                for (const auto& entryA : it->second) {
                    local_c[(entryA.row - row_start) * n + entryB.col] += entryA.value * entryB.value;
                }
            }
        }
        
        // the old recv_buffer_ring_topology now becomes the new send_buffer_ring_topology
        send_buffer_ring_topology.resize(recv_buffer_ring_topology.size());
        std::copy(recv_buffer_ring_topology.begin(), recv_buffer_ring_topology.end(), send_buffer_ring_topology.begin());
    }

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Time elapsed: %.6fs\n", end_time - start_time);
    }

    // gather a, b, c to rank 0
    uint64_t* total_a = (uint64_t*)calloc(n * n, sizeof(uint64_t));
    uint64_t* total_b = (uint64_t*)calloc(n * n, sizeof(uint64_t));
    uint64_t* total_c = (uint64_t*)calloc(n * n, sizeof(uint64_t));

    MPI_Gather(flattened_local_a, n * n / size, MPI_UINT64_T, total_a, n * n / size, MPI_UINT64_T, 0, MPI_COMM_WORLD);

    MPI_Gather(flattened_local_b, n * n / size, MPI_UINT64_T, total_b, n * n / size, MPI_UINT64_T, 0, MPI_COMM_WORLD);

    MPI_Gather(local_c, n * n / size, MPI_UINT64_T, total_c, n * n / size, MPI_UINT64_T, 0, MPI_COMM_WORLD);

    if (print_flag == 1 && rank == 0) {
        std::ofstream outputFile(output_file_name);
        for (uint64_t i = 0; i < n * n; i++) {
            outputFile << total_a[i];
            if ((i + 1) % n != 0) {
                outputFile << " ";
            } else {
                outputFile << std::endl;
            }
        }

        outputFile << std::endl;
        for (uint64_t i = 0; i < n * n; i++) {
            outputFile << total_b[i];
            if ((i + 1) % n != 0) {
                outputFile << " ";
            } else {
                outputFile << std::endl;
            }
        }

        outputFile << std::endl;
        for (uint64_t i = 0; i < n * n; i++) {
            outputFile << total_c[i];
            if ((i + 1) % n != 0) {
                outputFile << " ";
            } else {
                outputFile << std::endl;
            }
        }

        outputFile.close();
    }

    MPI_Finalize();
    return 0;
}
