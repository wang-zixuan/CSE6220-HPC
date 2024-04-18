# CSE 6220 Programming Assignment 3 Report

## Team members

Zixuan Wang, Jiashu Li, Yusen Su

## Evaluation of our program

### 1. Plot for: $p=16, e=0.01,$ three different $n$

### 2. Plot for: fix $n \geq 10000$ and change $e=0.1, 0.01, 0.001$ using $p=2,4,8,16$


### 3. Space analysis of our program

Suppose the sizes of matrix A and B are both $n\times n$ and we have $p$ processors. The sparsity parameter is $s$.

On each processor, we store $n/p$ rows of A, B, and C with sparsity $s$, so the space complexity on each processor will be $O(3s\cdot \frac{n^2}{p})$.

In order to transpose B, we need to create local send buffers and receive buffers for count and data, which requires $O(2\cdot p + 2\cdot s\cdot \frac{n^2}{p})$ space.

To rotate the B matrix using ring topology, we create a send and receive buffer with $O(2\cdot s\cdot \frac{n^2}{p})$ space. 

If we need to store the data into an output file, 

### 4. Runtime analysis of our program

### 5. Empirical analysis, observations, and conclusion

(Theoretically analyze the runtime of your parallel algorithm and discuss if the experimental results supports your analysis or not. Provide explanations if your experimental performance doesn’t match your theoretical analysis. 

Try to come up with some important observations on your program. We are not asking that specific questions be answered but you are expected to think on your own, experiment and show the conclusions along with the evidence that led you to reach the conclusions. Any irregular or unexpected results should also be noted and explained why it is unavoidable. Include your plots and observations in a PDF file with name report.pdf.)



### 6. Contributions of each team member
| Team member | Contribution |
| :------------------: | :----------: |
|  Zixuan Wang  | Implementation of George's algorithm |
|  Jiashu Li    |  |
|  Yusen Su     |  |