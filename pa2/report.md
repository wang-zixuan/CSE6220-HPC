# CSE 6220 Programming Assignment 2 Report

## Team members

Jiashu Li, Yusen Su, Zixuan Wang

## Space and run-time analysis of our implementation (4 pts)

### Arbitrary permutation
Local Variables: The space taken by local primitive variables (process_rank, num_processes, sendtype_size, recvtype_size, total_requests, and offset) is constant, O(1).

MPI_Request and MPI_Status Arrays: The function allocates two arrays of size 2 * (num_processes - 1) for MPI_Request and MPI_Status objects. Since the size of these arrays depends linearly on the number of processes (num_processes), the space complexity for these arrays is O(P), where P is the number of processes.

Buffers (sendbuf and recvbuf): The space needed for the send and receive buffers is not allocated within this function but is a critical part of the overall memory usage. Assuming the size of the data being sent and received is proportional to the number of processes, the space complexity related to these buffers would be O(N * P) for each, where N is the size of the data per process.

Therefore, the total space complexity of the function is $O(P + N * P)$, considering both the internal data structures and the input/output buffers.

Two main factors in the communication operation result in $O(τ⋅p+μ⋅m⋅p)$ for the runtime of the arbitrary all-to-all function. The τ⋅p term arises from the fact that each process sends a message to every other process, resulting in a latency cost τ for each of the p processes involved. Second, the μ⋅m⋅p term is introduced by the actual data transmission, which happens p times (once to each of the other processes), with each process sending a message of size m. This term accounts for the total volume of data transmitted across the network and scales with the size of the data and the number of processes. The total time cost of the algorithm is represented by the sum of these two terms, which includes both the one-time overhead associated with initiating message transfers and the continuous cost of data transmission.

### Hypecubic permutation

### Matrix transpose

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
