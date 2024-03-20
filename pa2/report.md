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

Different matrix sizes are being tested for the three different algorithms, the sizes are 160, 320, 640, 1280, 2560, 5120.

### $p = 8$

| Matrix Size | Algorithm Type | Time      |
|-------------|----------------|-----------|
|160	|a|	0.000118|
|320	|a	|0.001253|
|640	|a	|0.003189|
|1280	|a	|0.008252|
|2560	|a	|0.029195|
|5120	|a	|0.164851|
|160	|h	|0.000162|
|320	|h	|0.000775|
|640	|h	|0.002782|
|1280	|h	|0.010888|
|2560	|h	|0.044281|
|5120	|h	|0.222264|
|160	|m	|0.000914|
|320	|m	|0.001241|
|640	|m	|0.002923|
|1280	|m	|0.007609|
|2560	|m	|0.028734|
|5120	|m	|0.163553|


![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/88a0f0db-2806-4445-9360-b8fd0e504e38)




### $p = 16$

| Matrix Size | Algorithm Type | Time      |
|-------------|----------------|-----------|
|160	|a	|0.001608|
|320	|a	|0.000305|
|640	|a	|0.000843|
|1280	|a	|0.005892|
|2560	|a	|0.016120|
|5120	|a	|0.071288|
|160	|h	|0.000277|
|320	|h	|0.000430|
|640	|h	|0.001478|
|1280	|h	|0.005755|
|2560	|h	|0.023941|
|5120	|h	|0.108595|
|160	|m	|0.001591|
|320	|m	|0.000246|
|640	|m	|0.000845|
|1280	|m	|0.005225|
|2560	|m	|0.016170|
|5120	|m	|0.073887|


![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/8b56c0f4-9fc8-406f-bc4f-812e3dd90096)


### Empirical analysis, observations, and conclusion (8 pts)
Both for 8 and 16 processors, we can observe.
#### Key observations: 
-- As the matrix size increases, all three algorithms exhibit an increase in execution time, which makes sense given the increased volume of data that needs to be transferred and processed. This non-linear scaling behavior is eloquently demonstrated by the plot's logarithmic scales for both axes.

-- The execution times of the algorithms differ very little at smaller matrix sizes (160 to 640). However, notable variations appear as the matrix size grows. While the MPI all-to-all (m) and arbitrary all-to-all (a) algorithms perform similarly at all sizes, the hypercubic all-to-all (h) algorithm typically executes more slowly, especially at larger matrix sizes. This shows that the other two methods scale better than the hypercubic approach, which might involve more complicated communication patterns or overheads.

-- The MPI all-to-all algorithm performs marginally better than the arbitrary approach, even though the execution times of the two algorithms are similar (a and m). This might be the result of the MPI library's optimization for data communication, indicating that using MPI's built-in techniques could boost efficiency for large-scale data communication.

-- Unexpected outcomes: The noteworthy finding in the given data is the hypercubic algorithm (h)'s relativeperformance at larger scales; there are no overtly irregular results. Because hypercubic and other specialized algorithms have more structured communication patterns, one might expect them to perform better. On systems with a limited number of processors (8 in this case), however, the overhead of organizing these patterns may exceed their theoretical communication efficiency. One explanation for the observed slower speed of the hypercubic all-to-all (h) approach compared to the arbitrary (a) approach, particularly at larger scales, could be the extra computational overhead associated with the hypercubic method. In particular, each round of communication in the hypercubic approach adds an extra $O(n)$ computational overhead for data movement. This means that additional time is needed to move or rearrange data in order to follow the hypercubic communication pattern, on top of the time needed for data transfer itself. 

#### Comparing 8 and 16 processor runtimes: 
Analyzing the execution time trends of programs running on eight and sixteen processors, respectively, reveals several important conclusions. In general, all three algorithms (a for arbitrary, h for hypercubic, and m for MPI) show a notable reduction in execution time when the number of processors is doubled from 8 to 16. The benefits of parallel processing are illustrated by this improvement, which shows how multiplying the number of processors can significantly speed up the calculation, particularly when working with large matrix sizes.

The performance benefit of adding processors is especially noticeable for the MPI all-to-all (m) and arbitrary all-to-all (a) communication strategies at larger matrix sizes. This indicates that these algorithms are well-suited to increasing processor counts, making use of the extra capacity to shorten execution times overall. Although the hypercubic (h) algorithm performs better when there are more processors, it still lags behind the other two algorithms in terms of execution time reduction. This could be because the hypercubic (h) algorithm has inherent additional computational overhead for data movement, which may not scale as well as the number of processors.

In conclusion, the algorithms' scalability is demonstrated by the increase in processor count from 8 to 16, all of which show shorter execution times but varied degrees of efficiency gains. This comparison emphasizes how, in order to fully utilize the potential of higher processor counts, parallel algorithms must be optimized for both computation and communication.

## Contributions of each team member
| Team member | Contribution |
| :------------------: | :----------: |
|  Jiashu Li  | Arbitrary all-to-all implementation & experiments |
|  Yusen Su |  Hypercubic all-to-all implementation   |
| Zixuan Wang  | MPI_Alltoall & logic of matrix transpose in parallel & experiments |
