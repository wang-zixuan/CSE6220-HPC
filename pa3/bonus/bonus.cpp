#include "mpi.h"
#include <string>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <fstream>

struct SparseMatrixEntry {
    uint64_t row = -1;
    uint64_t col;
    uint64_t value;
};

MPI_Datatype MPI_SPARSE_ENTRY;

bool compareByCol(const SparseMatrixEntry& a, const SparseMatrixEntry& b) {
    return a.col < b.col;
}

// Function to compare SparseMatrixEntry by 'row'
bool compareByRow(const SparseMatrixEntry& a, const SparseMatrixEntry& b) {
    return a.row < b.row;
}

std::vector<SparseMatrixEntry> generateSparseMatrix(uint64_t n, double sparsity, int my2drank) {
    // generate values for n/p rows and n cols
    // seed for sparsity
    auto seed = std::chrono::system_clock::now().time_since_epoch().count() + my2drank;
    std::mt19937 rngSparsity(static_cast<unsigned int>(seed));
    std::uniform_real_distribution<double> distSparsity(0.0, 1.0);

    auto value_seed = std::chrono::system_clock::now().time_since_epoch().count() / (my2drank + 1);
    std::mt19937_64 rngValue(static_cast<unsigned int>(value_seed));
    std::uniform_int_distribution<uint64_t> distValue(0, 10);

    std::vector<SparseMatrixEntry> sparseMatrix;

    for (uint64_t i = 0; i < n; i++) {
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
    int dims[2], periods[2];

    int my2drank, mycoords[2];
    int uprank, downrank, leftrank, rightrank, coords[2];
    int shiftsource, shiftdest;

    uint64_t n;
    double sparsity_parameter;
    int print_flag;
    std::string output_file_name;

    double start_time, end_time;

    MPI_Comm comm_2d;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    n = std::stoull(argv[1]);
    sparsity_parameter = std::stod(argv[2]);
    print_flag = std::stoi(argv[3]);
    output_file_name = argv[4];

    const int nitems = 3;
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_UINT64_T, MPI_UINT64_T, MPI_UINT64_T};

    MPI_Aint offsets[3];
    offsets[0] = offsetof(SparseMatrixEntry, row);
    offsets[1] = offsetof(SparseMatrixEntry, col);
    offsets[2] = offsetof(SparseMatrixEntry, value);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_SPARSE_ENTRY);
    MPI_Type_commit(&MPI_SPARSE_ENTRY);

    dims[0] = dims[1] = std::sqrt(size);
    periods[0] = periods[1] = 1;

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm_2d);

    MPI_Comm_rank(comm_2d, &my2drank);
    MPI_Cart_coords(comm_2d, my2drank, 2, mycoords);

    MPI_Cart_shift(comm_2d, 1, -1, &rightrank, &leftrank);
    MPI_Cart_shift(comm_2d, 0, -1, &downrank, &uprank);

    uint64_t n_local = n / dims[0];

    std::vector<SparseMatrixEntry> sparseMatrixA = generateSparseMatrix(n_local, sparsity_parameter, my2drank);
    std::vector<SparseMatrixEntry> sparseMatrixB = generateSparseMatrix(n_local, sparsity_parameter, my2drank);

    uint64_t* local_c = (uint64_t*)calloc(n_local * n_local, sizeof(uint64_t));

    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    

    std::sort(sparseMatrixA.begin(), sparseMatrixA.end(), compareByCol);
    std::sort(sparseMatrixB.begin(), sparseMatrixB.end(), compareByRow);

    // initial matrix alignment for A
    MPI_Cart_shift(comm_2d, 1, -mycoords[0], &shiftsource, &shiftdest);

    int send_size_a = sparseMatrixA.size();
    int recv_size_a;

    // communicate count first
    MPI_Sendrecv(&send_size_a, 1, MPI_INT, shiftdest, 0, &recv_size_a, 1, MPI_INT, shiftsource, 0, comm_2d, MPI_STATUS_IGNORE);
    std::vector<SparseMatrixEntry> recv_buffer_a(recv_size_a);
    MPI_Sendrecv(sparseMatrixA.data(), send_size_a, MPI_SPARSE_ENTRY, shiftdest, 0, recv_buffer_a.data(), recv_size_a, MPI_SPARSE_ENTRY, shiftsource, 0, comm_2d, MPI_STATUS_IGNORE);

    // initial matrix alignment for B
    MPI_Cart_shift(comm_2d, 0, -mycoords[1], &shiftsource, &shiftdest);
    int send_size_b = sparseMatrixB.size();
    int recv_size_b;

    // communicate count first
    MPI_Sendrecv(&send_size_b, 1, MPI_INT, shiftdest, 0, &recv_size_b, 1, MPI_INT, shiftsource, 0, comm_2d, MPI_STATUS_IGNORE);

    std::vector<SparseMatrixEntry> recv_buffer_b(recv_size_b);
    MPI_Sendrecv(sparseMatrixB.data(), send_size_b, MPI_SPARSE_ENTRY, shiftdest, 0, recv_buffer_b.data(), recv_size_b, MPI_SPARSE_ENTRY, shiftsource, 0, comm_2d, MPI_STATUS_IGNORE);

    for (int i = 0; i < dims[0]; i++) {

        for (const auto& entryB : recv_buffer_b) {
            auto lower = std::lower_bound(recv_buffer_a.begin(), recv_buffer_a.end(), entryB,
                                    [](const SparseMatrixEntry& entry, const SparseMatrixEntry& val) {
                                        return entry.col < val.row;
                                    });
            // Iterate through all entries with matching 'col' (now aligned with 'row' of recv_buffer)
            for (auto it = lower; it != recv_buffer_a.end() && it->col == entryB.row; ++it) {
                local_c[it->row * n_local + entryB.col] += it->value * entryB.value;
            }
        }

        // shift matrix A left by one
        send_size_a = recv_buffer_a.size();
        int cur_recv_size_a;
        MPI_Sendrecv(&send_size_a, 1, MPI_INT, leftrank, 0, &cur_recv_size_a, 1, MPI_INT, rightrank, 0, comm_2d, MPI_STATUS_IGNORE);

        std::vector<SparseMatrixEntry> cur_recv_buffer_a(cur_recv_size_a);
        MPI_Sendrecv(recv_buffer_a.data(), send_size_a, MPI_SPARSE_ENTRY, leftrank, 0, cur_recv_buffer_a.data(), cur_recv_size_a, MPI_SPARSE_ENTRY, rightrank, 0, comm_2d, MPI_STATUS_IGNORE);

        // copy cur_recv_buffer_a to recv_buffer_a
        recv_buffer_a.resize(cur_recv_size_a);
        std::copy(cur_recv_buffer_a.begin(), cur_recv_buffer_a.end(), recv_buffer_a.begin());

        // shift matrix B up by one
        send_size_b = recv_buffer_b.size();
        int cur_recv_size_b;
        MPI_Sendrecv(&send_size_b, 1, MPI_INT, uprank, 0, &cur_recv_size_b, 1, MPI_INT, downrank, 0, comm_2d, MPI_STATUS_IGNORE);

        std::vector<SparseMatrixEntry> cur_recv_buffer_b(cur_recv_size_b);
        MPI_Sendrecv(recv_buffer_b.data(), send_size_b, MPI_SPARSE_ENTRY, uprank, 0, cur_recv_buffer_b.data(), cur_recv_size_b, MPI_SPARSE_ENTRY, downrank, 0, comm_2d, MPI_STATUS_IGNORE);

        // copy cur_recv_buffer_b to recv_buffer_b
        recv_buffer_b.resize(cur_recv_size_b);
        std::copy(cur_recv_buffer_b.begin(), cur_recv_buffer_b.end(), recv_buffer_b.begin());
    }

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Time elapsed: %.6fs\n", end_time - start_time);
    }

    if (print_flag == 1) {
        int row_start_local = mycoords[0] * n_local;
        int col_start_local = mycoords[1] * n_local;
        SparseMatrixEntry* flattened_local_a = (SparseMatrixEntry*)calloc(n_local * n_local, sizeof(SparseMatrixEntry));
        SparseMatrixEntry* flattened_local_b = (SparseMatrixEntry*)calloc(n_local * n_local, sizeof(SparseMatrixEntry));
        SparseMatrixEntry* flattened_local_c = (SparseMatrixEntry*)calloc(n_local * n_local, sizeof(SparseMatrixEntry));

        for (const auto& entry : sparseMatrixA) {
            uint64_t row = entry.row;
            uint64_t col = entry.col;
            SparseMatrixEntry s;
            s.row = row_start_local + row;
            s.col = col_start_local + col;
            s.value = entry.value;
            flattened_local_a[row * n_local + col] = s;
        }

        for (const auto& entry : sparseMatrixB) {
            uint64_t row = entry.row;
            uint64_t col = entry.col;
            SparseMatrixEntry s;
            s.row = row_start_local + row;
            s.col = col_start_local + col;
            s.value = entry.value;
            flattened_local_b[row * n_local + col] = s;
        }

        for (uint64_t i = 0; i < n_local; i++) {
            for (uint64_t j = 0; j < n_local; j++) {
                SparseMatrixEntry s;
                s.row = row_start_local + i;
                s.col = col_start_local + j;
                s.value = local_c[i * n_local + j];
                flattened_local_c[i * n_local + j] = s;
            }
        }

        SparseMatrixEntry* total_a = NULL;
        SparseMatrixEntry* total_b = NULL;
        SparseMatrixEntry* total_c = NULL;

        uint64_t* total_a_num = NULL;
        uint64_t* total_b_num = NULL;
        uint64_t* total_c_num = NULL;

        if (my2drank == 0) {
            total_a = (SparseMatrixEntry*)calloc(n * n, sizeof(SparseMatrixEntry));
            total_b = (SparseMatrixEntry*)calloc(n * n, sizeof(SparseMatrixEntry));
            total_c = (SparseMatrixEntry*)calloc(n * n, sizeof(SparseMatrixEntry));

            total_a_num = (uint64_t*)calloc(n * n, sizeof(uint64_t));
            total_b_num = (uint64_t*)calloc(n * n, sizeof(uint64_t));
            total_c_num = (uint64_t*)calloc(n * n, sizeof(uint64_t));
        }

        MPI_Gather(flattened_local_a, n_local * n_local, MPI_SPARSE_ENTRY, total_a, n_local * n_local, MPI_SPARSE_ENTRY, 0, comm_2d);
        MPI_Gather(flattened_local_b, n_local * n_local, MPI_SPARSE_ENTRY, total_b, n_local * n_local, MPI_SPARSE_ENTRY, 0, comm_2d);
        MPI_Gather(flattened_local_c, n_local * n_local, MPI_SPARSE_ENTRY, total_c, n_local * n_local, MPI_SPARSE_ENTRY, 0, comm_2d);

        if (my2drank == 0) {
            for (uint64_t i = 0; i < n * n; i++) {
                if (total_a[i].row != -1) {
                    total_a_num[total_a[i].row * n + total_a[i].col] = total_a[i].value;
                }
            }

            for (uint64_t i = 0; i < n * n; i++) {
                if (total_b[i].row != -1) {
                    total_b_num[total_b[i].row * n + total_b[i].col] = total_b[i].value;
                }
            }

            for (uint64_t i = 0; i < n * n; i++) {
                total_c_num[total_c[i].row * n + total_c[i].col] = total_c[i].value;
            }

            std::ofstream outputFile(output_file_name);
            for (uint64_t i = 0; i < n * n; i++) {
                outputFile << total_a_num[i];
                if ((i + 1) % n != 0) {
                    outputFile << " ";
                } else {
                    outputFile << std::endl;
                }
            }

            outputFile << std::endl;
            for (uint64_t i = 0; i < n * n; i++) {
                outputFile << total_b_num[i];
                if ((i + 1) % n != 0) {
                    outputFile << " ";
                } else {
                    outputFile << std::endl;
                }
            }

            outputFile << std::endl;
            for (uint64_t i = 0; i < n * n; i++) {
                outputFile << total_c_num[i];
                if ((i + 1) % n != 0) {
                    outputFile << " ";
                } else {
                    outputFile << std::endl;
                }
            }

            outputFile.close();
        }

    }

    MPI_Comm_free(&comm_2d);
    MPI_Finalize();
    return 0;

}