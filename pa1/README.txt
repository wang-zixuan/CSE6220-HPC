This program calculates the value of pi using the Monte Carlo method with MPI for parallel processing.

Below is the workflow of the program:
1) The processor with rank 0 takes n as the total number of points used for the estimation. 

2) n points will be distributed across MPI processes, and each process generates pairs of random numbers within the [0, 1] range to simulate points in a unit square, determining if they fall inside a quarter circle.

3) The results are then aggregated to the processor with rank 0 by MPI_reduce to estimate pi.

The machine we used for generating the results:
We ran our code on PACE-ICE. We allocated 64 processes and then tested our algorithm by passing different parameters to srun (see job-sbatch). The output is in slurm-273389.out. PACE-ICE automatically allocated 4 nodes with 64 cpus for us.