CONTRIBUTIONS

TODO: write a brief summary of how each team member contributed to
the project.

Jiaqi Yu: Completed Tasks 1 and 2
Lily Ru: Completed Tasks 3 and 4
Collectively reviewed the code and experiment results

REPORT

TODO: add your report according to the instructions in the
"Experiments and analysis" section of the assignment description.

Experimental Results
The results from performing the sort operation across various threshold values are summarized as follows:

Threshold	Real Time (s)	User Time (s)	System Time (s)
2097152     0.384 s	        0.370 s	        0.008 s
1048576	    0.213 s	        0.363 s	        0.034 s
524288	    0.150 s	        0.411 s	        0.040 s
262144	    0.121 s	        0.441 s	        0.037 s
131072	    0.114 s	        0.459 s	        0.047 s
65536	    0.110 s	        0.444 s	        0.100 s
32768	    0.118 s	        0.487 s	        0.111 s
16384	    0.124 s	        0.536 s	        0.149 s

The experimental results demonstrate the relationship between parallelization threshold and sorting performance for a 16MB dataset. At the highest threshold (2,097,152 elements), the program completed in 0.384s with minimal parallelism. As the threshold decreased, execution time improved significantly: 0.213s at 1,048,576, 0.150s at 524,288, and reaching an optimal 0.110s at 65,536 elements. However, below this threshold, performance began to degrade, with times increasing to 0.118s at 32,768 and 0.124s at 16,384 elements.

The performance improvements observed as the threshold decreased from 2,097,152 to 65,536 directly correlate with increased parallelism in the computation. The parsort program uses a divide-and-conquer approach where data is recursively split until chunks reach the threshold size, at which point sequential sorting occurs. When the threshold is high, few processes are created, meaning most computation executes sequentially in a single process. As the threshold decreases, more subdivisions occur before reaching the base case, creating more child processes that can execute concurrently. For instance, at threshold 65,536 with 16MB of data, approximately 32 parallel processes handle the base-case sorting operations. These independent sorting tasks are the primary components that the OS kernel can schedule across multiple CPU cores simultaneously, enabling true parallel execution and reducing wall-clock time.

The diminishing returns and eventual performance degradation below the 65,536 threshold can be explained by process creation overhead. At threshold 16,384, approximately 128 processes are created for base-case sorting. While this maximizes parallelism potential, the overhead of forking processes, managing inter-process communication, and context switching between numerous processes begins to outweigh the benefits of parallel execution. Additionally, if the system has fewer CPU cores than active processes, excessive context switching occurs as the kernel time-slices CPU access among competing processes. The merge operations that combine sorted subarrays must also wait for child processes to complete, and with many small processes, the synchronization overhead accumulates. The optimal threshold of 65,536 represents the balance point where sufficient parallelism is achieved to utilize available CPU cores without incurring excessive process management overhead.
