George's algorithm:

1. How our program works

1.1 Workflow

1.1.1 Generate random data for matrix A and matrix B

The first step of our algorithm is to generate (n / p) rows of matrix A and B on each processor. To implement this, we define two random number generator separately for sparsity and value. If the generated random value is smaller then the sparsity, we will generate a random value for that entry. We set the range of value to be (0, UINT16_MAX) to ensure that we won't have overflow during matrix-matrix multiplication.

1.1.2 Transpose B such that each processor stores (n / p) columns of B 

For every entry of B in a single processor, we find its target processor based on the column index. Then, we use MPI_Alltoall to communicate the count first and MPI_Alltoallv to communicate the real data. We define a MPI datatype called MPI_SPARSE_ENTRY for our SparseMatrixEntry struct. After this, each processor will store (n / p) columns of B.

1.1.3 Rotate the B matrix using the ring topology such that all rows of A can perform dot products with all the columns of B 

To implement this, we firstly compute initial values in C using local A and B on the same processor. In our algorithm, we traverse all the non-zero entries of A and B, and we only compute the dot product when the column index of A is equal to the row index of B.

Then, a processor will receive data of (n / p) columns of B from its left processor and send (n / p) columns of B to its right processor. The process will be performed (size - 1) times. In every iteration, we will add the value of dot product of A and B into local C. This ensures that (n / p) rows of A can perform dot product on all columns of B. 

1.1.4 Print results

If the print flag is equal to 1, we will gather results from all processors and store A, B, and C as dense matrices, then store them into an output file.

2. Machine used for generating the results

We run our code on pace-ice with 16 processes allocated. Below is the detail of the machine information:

Resources:     cpu=16,mem=64G,node=3
Rsrc Used:     cput=00:15:12,vmem=4560K,walltime=00:00:57,mem=4552K,energy_used=0
Partition:     coc-cpu
Nodes:         atl1-1-02-003-19-[1-2],atl1-1-02-003-20-1

Bonus algorithm:

