# CSE 6220 Programming Assignment 2 Report

## Team members

Jiashu Li, Yusen Su, Zixuan Wang

## Space and run-time analysis of our implementation (4 pts)

### Arbitrary permutation
#### Space Complexity:
Local Variables: The space taken by local primitive variables (process_rank, num_processes, sendtype_size, recvtype_size, total_requests, and offset) is constant, O(1).

MPI_Request and MPI_Status Arrays: The function allocates two arrays of size 2 * (num_processes - 1) for MPI_Request and MPI_Status objects. Since the size of these arrays depends linearly on the number of processes (num_processes), the space complexity for these arrays is O(P), where P is the number of processes.

Buffers (sendbuf and recvbuf): The space needed for the send and receive buffers is not allocated within this function but is a critical part of the overall memory usage. Assuming the size of the data being sent and received is proportional to the number of processes, the space complexity related to these buffers would be O(N * P) for each, where N is the size of the data per process.

Therefore, the total space complexity of the function is $O(P + N * P)$, considering both the internal data structures and the input/output buffers.

#### Runtime:
Two main factors in the communication operation result in $O(τ⋅p+μ⋅m⋅p)$ for the runtime of the arbitrary all-to-all function. The τ⋅p term arises from the fact that each process sends a message to every other process, resulting in a latency cost τ for each of the p processes involved. Second, the μ⋅m⋅p term is introduced by the actual data transmission, which happens p times (once to each of the other processes), with each process sending a message of size m. This term accounts for the total volume of data transmitted across the network and scales with the size of the data and the number of processes. The total time cost of the algorithm is represented by the sum of these two terms, which includes both the one-time overhead associated with initiating message transfers and the continuous cost of data transmission.

### Hypecubic permutation
#### Space Complexity:
The HPC_Alltoall_H function has an O(S) space complexity, where S is the send buffer size, which is calculated by multiplying the sendcount by the number of processes. The dynamic allocation of an integer array to hold send data and a local buffer used at each stage of the communication loop are taken into account by this space complexity. The dominating term of T_comm * log(size) represents the time complexity of the send and receive operations across the network. The time complexity is determined by adding the initialization step O(S), the computational work in the logarithmic number of steps O(S/2 * log(size)), and the communication time per step. Thus, O(S + (S/2) * log(size) + T_comm * log(size)) is the final runtime complexity, with the communication time probably being the most important component.
#### Runtime: 


### Matrix transpose
#### Pre_Transpose function:
The transposed matrix is stored in additional space. As each element of the input matrix is transposed into the transposed vector, the space complexity, which is O(block_size), is proportional to the size of the input matrix. There are two nested loops, which adds to the time complexity. Row_size iterations are executed by the inner loop and col_size iterations by the outer loop. The total number of iterations is block_size / n * n, which simplifies to block_size since row_size is block_size / n and col_size is n. O(block_size) is the time complexity as a result.

#### Post_transpose function:
This function generates a new vector res to hold the transposed matrix, much like pre_transpose does. The amount of space needed for res is O(size * col_size * col_size), which is proportional to the number of elements in the input matrix. The space complexity reduces to O(n * col_size) = O(n^2 / size) since n = size * col_size. This function has two nested loops that run size times and col_size times, respectively, in terms of time complexity. A call to vector::insert, which inserts k elements in O(k) time, is made inside the inner loop. A time complexity of O(size * col_size^2) results from the handling of col_size elements by each insert operation, and size such operations for each iteration of the outer loop. Once more, this simplifies to O(n * col_size) = O(n^2 / size) since n = size * col_size.
## Evaluate the performance with varying input matrix sizes

(Fixing the number of processors to 8 and 16, plot the runtimes of the three approaches over different matrix sizes and describe your observations of the results. 2 plots, 8 pts)

### $p = 8$

? problem size: 

### $p = 16$

? problem size: 

### Empirical analysis, observations, and conclusion (8 pts)

(Try to come up with some important observations on your program. We are not asking that specific questions be answered but you are expected to think on your own, experiment and show the conclusions along with the evidence that led you to reach the conclusions. Any irregular or unexpected results should also be noted and explained why it is unavoidable)

## Contributions of each team member
| Team member | Contribution |
| :------------------: | :----------: |
|  Jiashu Li  | Arbitrary all-to-all implementation & experiments |
|  Yusen Su |  Hypercubic all-to-all implementation   |
| Zixuan Wang  | MPI_Alltoall & logic of matrix transpose in parallel & experiments |
