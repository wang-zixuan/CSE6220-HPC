# CSE 6220 Programming Assignment 3 Report

## Team members

Yusen Su, Jiashu Li, Zixuan Wang

## Evaluation of George's algorithm (we use `-O3` to optimize the compilation)

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

## Bonus algorithm (we also use `-O3` to optimize the compilation)

### 1. Plot for: $p=16, e=0.01,$ three different $n$
![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/aa374e68-7833-455f-8e0b-7543ca730108)

### 2. Plot for: fix $n \geq 10000$ and change $e=0.1, 0.01, 0.001$ using $p=4,16$
![image](https://github.com/wang-zixuan/CSE6220-HPC/assets/99767753/e9a13a5a-d92f-4e40-adb1-65870e1859ed)

### 3. Runtime analysis
The frist several steps are similar to the George's algorithm described above.

3.1. Generate Sparse Matrices A and B
- Each processor generates its portion of the sparse matrices. Given the matrix size $n$ divided among the processors in a 2D grid: the runtime is $O(\frac{n^2}{p})$ where $p$ is the number of processors and the matrices are $n \times n$.

- Each process sends and receives data for matrix alignment using MPI\_Sendrecv. The complexity involves:

    communication cost: $O(\tau + \mu \cdot s \cdot \frac{n^2}{p})$
 
    where $\tau$ is the latency, $\mu$ is the inverse of bandwidth, and $s$ is the sparsity that affects the number of non-zero elements communicated.

3.2. Initial Alignment

Initial alignment involves shifting matrix A and B to align them for the multiplication.

- Each process sends and receives data for matrix alignment using `MPI_Sendrecv`. The complexity involves:
    communication cost $O(\tau + \mu \cdot s \cdot \frac{n^2}{p})$
    where $\tau$ is the latency, $\mu$ is the inverse of bandwidth, and $s$ is the sparsity that affects the number of non-zero elements communicated.

3.3. Sorting
- Both matrices are sorted in preparation for multiplication.
$O\left(\frac{sn^2}{p} \log \frac{sn^2}{p}\right)$
This is assuming sorting is done locally on each process with the complexity based on the sparsity and the size of the local matrix slice.

3.4. Matrix Multiplication in a Shifted Grid
-The main computation involves multiplying portions of matrices A and B that each process holds after alignment and communication:
$O\left(\frac{sn^2}{p}\right)$
Each process iterates, performing multiplication and shifting matrices across the Cartesian topology.

3.5. Iterative Shifting and Multiplication
- Shift Matrix A: Left by one step in the Cartesian grid.
- Shift Matrix B: Up by one step in the Cartesian grid.
- The complexity for each shift involves `MPI_Sendrecv` operations and local computation for dot product:
$O\left((\sqrt{p}-1) \cdot (\tau + \mu \cdot \frac{sn^2}{p} + \frac{sn^2}{p})\right)$
    This continues for $\sqrt{p}-1$ steps (assuming the worst-case where each process needs to receive data from every other process directly or indirectly).

The total runtime of the algorithms is:
$O\left(\frac{n^2}{p} + \tau \log p + \mu p \log p + \frac{sn^2}{p} \log \frac{sn^2}{p} + (\sqrt{p}-1) \cdot (\tau + \mu \cdot \frac{sn^2}{p} + \frac{sn^2}{p})\right)$

### 4. Observations
#### Communication:
- George's Algorithm: Employs a simple method to align data across processes by distributing matrices among processors in an undefined structure. Since data must be requested dynamically depending on computation requirements, this initial misalignment may result in increased communication overhead during the multiplication phase.
  
- Bonus Algorithm: Uses a two-dimensional Cartesian grid to arrange data and processes. Data locality and predictable communication patterns are made possible by this structured distribution. It minimizes the amount of data communicated during the computational phase by aligning matrices prior to multiplication.

#### Matrix Multiplication:
- George's Algorithm: Multiplies without first aligning the matrices, which could result in delays for each processor as it may need to wait for data from other processors if it is not locally available.
- Bonus Algorithm: Makes use of an initial alignment step to guarantee that, prior to the multiplication, each processor has the required submatrices. By doing this, runtime delays brought on by waiting for data during the multiplication stage are decreased.

#### Why the bonus algorithm is faster?

- The overhead related to data transfers is reduced by using a 2D Cartesian grid and preset communication patterns (as opposed to dynamic data requests). The communication cost is maximized since every process is aware of the precise location and time of data transmission and reception.

#### Observation and Conclusion
- The Bonus Algorithm performs better than George's at all tested sparsity levels, with the performance gap growing noticeably as the sparsity decreases, according to a comparative analysis between the two algorithms across a range of sparsity levels. In particular, the Bonus Algorithm continues to have a discernible advantage at higher sparsities (e.g., e=0.1), even though both algorithms encounter longer computation times as a result of the increased number of non-zero elements. This benefit is significantly increased at lower sparsity levels (e.g., e=0.001 and e=0.01), where the Bonus Algorithm can handle sparse matrices far faster than George's Algorithm due to its efficiency in handling computation and communication. The trend suggests that the Bonus Algorithm can be up to multiple times faster, especially in settings where matrix sparsity plays a major role in efficiency and computational overhead.
## Contributions of each team member
| Team member | Contribution |
| :------------------: | :----------: |
|  Yusen Su     | Implementation of bonus algorithm |
|  Jiashu Li    |  Result Analysis and Report Generation|
|  Zixuan Wang  | Implementation of George's algorithm |