1. How our program works

1.1 Workflow of our code

1.1.1 Read data from txt file
We read matrix data from txt file and store it into a 1D array, which is more convenient for later usage.

1.1.2 Scatter data into different processors
We distribute the rows evenly among all the processors. In detail, every processor will hold n / p rows of the matrix. 

1.1.3 Pre-process data for all-to-all communication
Since we store the data in a row-major way, but in matrix transpose, communication between processors needs to be done in a blockwise way. Say a processor has n/p x n elements (p submatrices with size of n/p x n/p), it should send a submatrice to the corresponding processor, and then perform the local transpose of each submatrice to get the final result. 

To enable this in a 1D array, we reorder the local array in a column-major way to ensure that the submatrice that should be kept in the same processor won't be sent to another processor.

For example, if we have 2 processors and a 4x4 matrix

A B | C D
E F | G H
----+-----
I J | K L 
M N | O P

After pre-processing, rank 0 will store [A, E, B, F, C, G, D, H], and rank 1 will store [I, M, J, N, K, O, L, P].

1.1.4 All-to-all communication
Then we perform the all-to-all communication based on the command line argument, and the details will be covered in the later sections.

After all-to-all communication, rank 0 will store [A, E, B, F, I, M, J, N], and rank 1 will store [C, G, D, H, K, O, L, P].

1.1.5 Post-process data to get final results
After all-to-all communication, the submatrices are still not transposed. The target is

A E | I M
B F | J N
----+-----
C G | K O
D H | L P

So we post-process (re-order) data here, and rank 0 will store [A, E, I, M, B, F, J, N], rank 1 will store [C, G, K, O, D, H, L, P].

1.1.6 Gather data to rank 0 and save results
To get the final results, we gather all the data into rank 0 to a 1D array and store data into a txt file in a row-major way.

1.2 Arbitrary all-to-all explanation
-- Function Logic: The HPC_Alltoall_A function uses MPI's immediate send (MPI_Isend) and receive (MPI_Irecv) operations to carry out all-to-all communication in a non-blocking manner. The logic of the function develops in multiple stages: 

-- Initialization: The function starts by counting the number of processes in the given communicator (comm) and the rank of the calling process.

-- Local Copy: The function copies some memory locally before initiating the communication. The data meant for the process is copied from the send buffer to the receive buffer in this step. This is required because every process sends data to itself in the all-to-all communication pattern.

Communication Loop: The function then enters a loop where it schedules non-blocking sends and receives for each other process in the communicator. For each pair of operations (send and receive):

Based on the current process rank and an offset, the send operation's destination and the receive operation's source are determined. This computation guarantees that every process exchanges information with every other process precisely once.
To start a non-blocking send to the calculated destination process, MPI_Isend is called.
To start a non-blocking receive from the calculated source process, MPI_Irecv is called right away.
MPI request objects kept in an array are used to track both operations, enabling the function to wait for all operations to finish before returning.
Completion: The function uses MPI_Waitall to wait for each send and receive operation to finish after scheduling them all. This call ensures that all communication is completed before the function returns by blocking the calling process until all designated non-blocking operations have been completed.

1.3 Hypercubic all-to-all explanation

2. Machine used for generating the results

We run our code on pace-ice with 16 processes allocated. Below is the detail of the machine information:

Resources:     cpu=16,mem=64G,node=1
Rsrc Used:     cput=00:16:00,vmem=4296K,walltime=00:01:00,mem=4288K,energy_used=0
Partition:     coc-cpu
Nodes:         atl1-1-02-010-8-1
