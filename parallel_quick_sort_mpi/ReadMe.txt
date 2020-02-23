/************************How to compile and run *****************************/
0. the turnin includes a lot of other files that I didn't intend to submit.  To be clear , my home work contains: 
	parallel_qsort_mpi.c
	Makefile
	ReadMe.txt
1. use 'make' to compile the file
2. use 'mpirun -n [number of processes you want to use] output.out [lengh of the array]' to run the program

/***********************Explanation of the program***************************/
1. process 0 (MASTER) is the master process, and the others are 'worker' process
2. master process will do all the 'extra' work: array initialization, time record, scatter and gather the array to all the processes
3.the whole processes:
	a. master process do array initialization, and record current time.
	b. master process scatter the array to all processes
	c. now all processes have his own data. 
		i)  each process will sort his own part of array first
		ii) now the loop will start , processes will keep being paired and do the work until the whole array is done. eg. 16 processes in total, 0 is first paired with 8 (global rank), then with 4, then with 2, then with 1.
		iii)in each loop, the process i and pair_i will do: 
			* compute the global pivot
			* i send data that is larger than global pivot to pair_i
			* pair_i send data that is smaller than global pivot to i
			* i merge the two parts of data( so does pair_i)
			* split communicator
	d. now all processes has sorted his part of data, just gather the data back to master processes
	e. redord end time, report time
4. the reported time includes two parts: 1 takes scatter time into account, the other doesn't. you can use whichever you need.
