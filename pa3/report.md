# CSE 6220 Programming Assignment 3 Report

## Team members

Yusen Su, Jiashu Li, Zixuan Wang

## Evaluation of our program (we use `-O3` to optimize the compilation)

### 1. Plot for: $p=16, e=0.01,$ three different $n$
![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/e8891a28-69aa-4c6e-b73d-7137f5996554)


### 2. Plot for: fix $n \geq 10000$ and change $e=0.1, 0.01, 0.001$ using $p=2,4,8,16$
![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/1930df84-aebd-4922-88a7-7019f49a4f9f)



### 3. Space analysis of our program

We only consider the case when we don't need to store the result to an output file.

Suppose the sizes of matrix A and B are both $n\times n$ and we have $p$ processors. The sparsity parameter is $s$.

1. On each processor, we store $n/p$ rows of A and B with sparsity $s$, so the space complexity on each processor will be $O(2\cdot s\cdot \frac{n^2}{p})$. Matrix C is stored in dense format, which requires $O(\frac{n^2}{p})$ space.

2. In order to transpose B, we need to create local send buffers and receive buffers for count and data, and two displacement vectors for send and receive buffer separately, which requires $O(4\cdot p + 2\cdot s\cdot \frac{n^2}{p})$ space.

3. To rotate the B matrix using ring topology, we create a send and receive buffer with total $O(2\cdot s\cdot \frac{n^2}{p})$ space. 

The total space complexity on each processor would be $O(\frac{n^2}{p}+4p+6s\cdot \frac{n^2}{p})$.

### 4. Runtime analysis of our program

1. Generate sparse matrix A and B: the nested loop for generating sparse matrix on each processor requires $O(\frac{n^2}{p})$ computation time.

2. Transpose matrix B: firstly we need to create a send buffer for local matrix B and calculate how many entries of B needs to be sent to each processor. Then we can communicate the real data. 

- prepare send counts and send buffer: $O(s\cdot \frac{n^2}{p})$

- communicate the counts to all processors using MPI_Alltoall (assume hypercubic permutation): $O(\tau \log p + \mu p \log p)$
- compute displacements: $O(p)$
- perform matrix transpose using MPI_Alltoallv (assume hypercubic permutation): $O(\tau \log p + \mu \cdot s\cdot \frac{n^2}{p} \cdot p \cdot \log p)$ 

3. sort local matrix A and B and perform dot product 
- we use `std::sort` in our code, which takes $O(\frac{sn^2}{p} \log \frac{sn^2}{p})$. 

- dot product of matrix A and B: $O(\frac{sn^2}{p})$

4. Rotate B using ring topology

- We need to communicate $p - 1$ times. In every iteration, a processor receives data from its left processor and sends data to its right processor. We don't need to sort B in the iteration since we have already sorted it in the previous step. Communicating count takes $O(\tau + \mu)$, and communciating real data takes $O(\tau + \mu \frac{sn^2}{p})$. Then dot product of A and B takes $O(\frac{sn^2}{p})$. The total runtime would be $O((p - 1)\cdot ((\tau + \mu) + (\tau + \mu \frac{sn^2}{p}) + \frac{sn^2}{p})) = O((p-1)\cdot (\tau +\mu \frac{sn^2}{p}))$.

Adding this together, the total runtime would be $O(\frac{n^2}{p} + 2\tau \log p + \mu p \log p + p + \mu s n^2 \log p + \frac{sn^2}{p}\log \frac{sn^2}{p} + (p-1)\cdot (\tau +\mu \frac{sn^2}{p}))$.

### 5. Empirical analysis, observations, and conclusion
#### Empirical Results Overview
- We tested the implementation of parallel sparse matrix multiplication with varying numbers of processors ($p$), matrix sizes ($n$), and sparsity levels ($s$) in our experimental setup. Important empirical findings are summed up as follows: 

1. Scaling with Matrix Size: With $p = 16$ and $s = 0.01$, the computation time increased from 0.006317 seconds to 0.099309 seconds as $n$ increased from 2000 to 8000. This trend indicates a non-linear increase in line with the growing demands for computation and communication.
2. Effect of Sparsity: Reducing sparsity from 0.1 to 0.001 consistently lowered computation times for $n = 10000$, highlighting effective handling of non-zero elements and less communication overhead.
3. Processor Scalability: Increasing $p$ from 2 to 16 decreased the runtime from 54.629744 seconds to 3.814617 seconds, exhibiting strong scalability and efficient parallelization with $n = 10000$ and $s = 0.1$.

#### Theoretical vs. Empirical Analysis:
- Comparing Empirical and Theoretical Analysis: Based on the computational complexity, the following theoretical estimates for the runtime of our program were made:
$O\left(\frac{n^2}{p} + 2\tau \log p + \mu p \log p + p + \mu s n^2 \log p + \frac{sn^2}{p}\log \frac{sn^2}{p} + (p-1)\cdot (\tau +\mu \frac{sn^2}{p})\right)$
and the complexity of space: $O\left(\frac{n^2}{p}+4p+6s\cdot \frac{n^2}{p}\right).$ Here are some comparisons and things to think about:
1. Runtime Comparisons: Overall, the experimental results followed the theoretical scaling trends, but sometimes the actual runtimes exceeded the predictions, possibly because the MPI operation efficiencies were underestimated and the overheads were overestimated.
2. Space Complexity: The absence of memory overflow problems indicates that the requirements were sufficiently captured by our space complexity model.

#### Discrepancies and Adjustments
- Discrepancies were observed, particularly at higher $p$ values where the reduction in runtime was more significant than predicted. Potential reasons include:
1. MPI Overheads: These were less than projected, especially for group operations such as MPI\_Alltoallv.
2. Hardware Capabilities: Compared to models, the underlying hardware was more effective at handling parallel tasks.

Modifications to the theoretical framework could improve the terminology used to describe communication overhead and take hardware capability observations into account.

#### Observations and Conclusions

Important findings include: 
1. Efficiency at High Sparsity: We found that our program handles highly sparse matrices with significant runtime reductions as the sparsity decreases.
2. Load Balancing: The efficiency of the ring topology utilized for matrix $B$ rotation and the distribution of matrix entries are both highly significant

Finally, the empirical analysis verifies the efficiency and robust scalability of our parallel sparse matrix multiplication implementation across a range of processor counts and sparsity levels. Further improvements in data distribution and communication efficiency may be investigated in future work, particularly for very large matrices or high processor counts.


### 6. Contributions of each team member
| Team member | Contribution |
| :------------------: | :----------: |
|  Yusen Su     |  |
|  Jiashu Li    |  Result Analysis and Report Generation|
|  Zixuan Wang  | Implementation of George's algorithm |
