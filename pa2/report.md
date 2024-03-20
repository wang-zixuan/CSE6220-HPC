# CSE 6220 Programming Assignment 2 Report

## Team members

Jiashu Li, Yusen Su, Zixuan Wang

## Space and run-time analysis of our implementation

### Arbitrary permutation
#### Space Complexity:
Local Variables: The space taken by local primitive variables (process_rank, num_processes, sendtype_size, recvtype_size, total_requests, and offset) is constant, $O(1)$.

MPI_Request and MPI_Status Arrays: The function allocates two arrays of size `2 * (num_processes - 1)` for MPI_Request and MPI_Status objects. Since the size of these arrays depends linearly on the number of processes (num_processes), the space complexity for these arrays is $O(p)$, where $p$ is the number of processes.

Buffers (sendbuf and recvbuf): The space needed for the send and receive buffers is not allocated within this function but is a critical part of the overall memory usage. Assuming the size of the data being sent and received is proportional to the number of processes, the space complexity related to these buffers would be $O(N * p)$ for each, where $N = \frac{n}{p}\times n$ is the size of the data per process.

Therefore, the total space complexity of the function is $O(p + N * p)$, considering both the internal data structures and the input/output buffers.

#### Runtime:
Two main factors in the communication operation result in $O(\tau⋅p+\mu⋅m⋅p)$ for the runtime of the arbitrary all-to-all function. The $\tau⋅p$ term arises from the fact that each process sends a message to every other process, resulting in a latency cost $\tau$ for each of the p processes involved. Second, the $\mu⋅m⋅p$ term is introduced by the actual data transmission, which happens $p$ times (once to each of the other processes), with each process sending a message of size $m$. This term accounts for the total volume of data transmitted across the network and scales with the size of the data and the number of processes. The total time cost of the algorithm is represented by the sum of these two terms, which includes both the one-time overhead associated with initiating message transfers and the continuous cost of data transmission.

### Hypecubic permutation
#### Space Complexity:
For every processor, we first cast the `const void* sendbuf` to `const int* sendbuf_int`, and copy the data from `sendbuf` to `sendbuf_int`, which requires $O(\frac{n}{p}\times n)$ space complexity per processor.

Apart from that, the function create a local send buffer with size $O(\frac{n}{p}\times n\times \frac{1}{2})$ per processor since we need to transmit an array with contiguous memory during point-to-point communication. So the total space required for all processors is $O(\frac{n}{p}\times n\times p + \frac{n}{p}\times n\times \frac{1}{2}\times p) = O(n^2)$.

#### Runtime: 

The communication time for hypercubic permutation is $O(\tau \log p + \mu \frac{n^2}{p} \log p)$, where $O(\frac{n^2}{p})$ is the size of data being transmitted between two processors in every round.

The computation time in every round is $O(\frac{n^2}{2p} \times 2)$, since we need to copy data between local send buffer and sendbuf 2 times. 

Therefore, the total runtime is $O(\tau \log p + \mu \frac{n^2}{p} \log p + \frac{n^2}{p}) = O(\tau \log p + \mu \frac{n^2}{p} \log p)$.

### Matrix transpose
#### pre_transpose function:
- space complexity: on every processor, before all-to-all communication, data will be iterated in a col-major way. The space complexity will be $O(\frac{n^2}{p})$ on every processor, so the total space complexity is $O(n^2)$.

- time complexity: we have a nested for loop to traverse through the whole data, so the time complexity is $O(\frac{n^2}{p})$.

#### post_transpose function:
- space complexity: This function generates a new vector to reorder the local data on every processor. So the space complexity is $O(\frac{n^2}{p})$.

- time complexity: the function just simply traverses the whole data and reorder it into a 1D array that follow the order of a transposed 2D array, so the time complexity is also $O(\frac{n^2}{p})$.

## Evaluate the performance with varying input matrix sizes

Different matrix sizes are being tested for the three different algorithms, the sizes are 160, 320, 640, 1280, 2560, 5120. We intentionally choose large data size to ensure that the result will be more relative to the size of the problem.

### $p = 8$

| Matrix Size | Algorithm  | Time (s) |
| :---------: | :--------: | :------: |
|     160     | Arbitrary  | 0.000118 |
|             | Hypercubic | 0.000162 |
|             |    MPI     | 0.000914 |
|     320     | Arbitrary  | 0.001253 |
|             | Hypercubic | 0.000775 |
|             |    MPI     | 0.001241 |
|     640     | Arbitrary  | 0.003189 |
|             | Hypercubic | 0.002782 |
|             |    MPI     | 0.002923 |
|    1280     | Arbitrary  | 0.008252 |
|             | Hypercubic | 0.010888 |
|             |    MPI     | 0.007609 |
|    2560     | Arbitrary  | 0.029195 |
|             | Hypercubic | 0.044281 |
|             |    MPI     | 0.028734 |
|    5120     | Arbitrary  | 0.164851 |
|             | Hypercubic | 0.222264 |
|             |    MPI     | 0.163553 |




![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/88a0f0db-2806-4445-9360-b8fd0e504e38)




### $p = 16$

| Matrix Size | Algorithm  | Time (s) |
| :---------: | :--------: | :------: |
|     160     | Arbitrary  | 0.001608 |
|             | Hypercubic | 0.000277 |
|             |    MPI     | 0.001591 |
|     320     | Arbitrary  | 0.000305 |
|             | Hypercubic | 0.000430 |
|             |    MPI     | 0.000246 |
|     640     | Arbitrary  | 0.000843 |
|             | Hypercubic | 0.001478 |
|             |    MPI     | 0.000845 |
|    1280     | Arbitrary  | 0.005892 |
|             | Hypercubic | 0.005755 |
|             |    MPI     | 0.005225 |
|    2560     | Arbitrary  | 0.016120 |
|             | Hypercubic | 0.023941 |
|             |    MPI     | 0.016170 |
|    5120     | Arbitrary  | 0.071288 |
|             | Hypercubic | 0.108595 |
|             |    MPI     | 0.073887 |




![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/8b56c0f4-9fc8-406f-bc4f-812e3dd90096)


### Empirical analysis, observations, and conclusion
#### Key observations: 
- As the matrix size increases, all three algorithms exhibit an increase in execution time, which makes sense given the increased volume of data that needs to be transferred and processed. This non-linear scaling behavior is eloquently demonstrated by the plot's logarithmic scales for both axes.

- The execution times of the algorithms differ very little at smaller matrix sizes (160 to 640). However, notable variations appear as the matrix size grows. While the MPI all-to-all (m) and arbitrary all-to-all (a) algorithms perform similarly at all sizes, the hypercubic all-to-all (h) algorithm typically executes more slowly, especially at larger matrix sizes. This shows that the other two methods scale better than the hypercubic approach. This might be due to the fact that the number of processor is very small in our experiment, and the pattern of runtime would be the opposite as expected.

- The MPI all-to-all algorithm performs marginally better than the arbitrary approach, even though the execution times of the two algorithms are similar (a and m). This might be the result of the MPI library's optimization for data communication, indicating that using MPI's built-in techniques could boost efficiency for large-scale data communication.

- Unexpected outcomes: The noteworthy finding in the given data is the hypercubic algorithm (h)'s relative performance at larger scales. Apart from that, there are no overtly irregular results. Because hypercubic and other specialized algorithms have more structured communication patterns, one might expect them to perform better. On systems with a limited number of processors (8 in this case), however, the overhead of organizing these patterns may exceed their theoretical communication efficiency. One explanation for the observed slower speed of the hypercubic all-to-all (h) approach compared to the arbitrary (a) approach, particularly at larger scales, could be the extra computational overhead associated with the hypercubic method. In particular, each round of communication in the hypercubic approach adds an extra $O(\frac{n^2}{p})$ computational overhead for data movement. This means that additional time is needed to move or rearrange data in order to follow the hypercubic communication pattern, on top of the time needed for data transfer itself. 

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
