
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

#include "parallel_qsort.h"

/* parameters */
 int arr_size = 100000000;
 int thread_num = 16;
 int max_value = 1000;


/*  helper functions */

// fill the array with random values
void randomize(int* arr, int size);

// print array 
void printArr(int* arr, int size);

// return a deep copy of array
int* copyArr(int* arr, int size);

// compare the two array entry by entry
void compareArr(int* correct_arr, int* your_arr, int size);


void main() {
	/*   1. initialization             */
	printf("Please input the size of array: (range: 1 to 100000000)\n");
	scanf("%d", &arr_size);

	printf("Please input the number of threads you want to use : (range: 1 to 16)\n");
	scanf("%d", &thread_num);
	
	int* arr;
	arr = malloc(sizeof(int) * arr_size);

	/* check if the arr has been successfully created */
	if (!arr) {
		perror("Error allocating memory");
		abort();
	}
	
	printf("initializing array with %d random integer\n", arr_size);
	randomize(arr, arr_size);
	int* copy_arr = copyArr(arr, arr_size);


	/****************************************/
	/* pass1: sort the array using 1 thread*/
	/****************************************/
	struct timeval sthread_start_time, sthread_end_time; 
	gettimeofday(&sthread_start_time,NULL);
	printf("begin single quick sort\n*******************************************************\n\n\n");

	/* single thread q sort*/
	parallel_quicksort(copy_arr, arr_size, 1);

	/* get end time and report time taken*/
	gettimeofday(&sthread_end_time,NULL);
	time_t cpu_time_used1 = sthread_end_time.tv_sec - sthread_start_time.tv_sec;
	printf("single thread quick sort took %ld seconds to execute \n\n\n\n", cpu_time_used1);



	/*******************************************************/
	/* pass2: sort the array using required num of threads */
	/*******************************************************/
	/* get start time*/
	gettimeofday(&sthread_start_time,NULL);
	printf("begin parallel quick sort using %d threads\n*******************************************************\n\n\n", thread_num);

	/* threaded q sort*/
	parallel_quicksort(arr, arr_size, thread_num);

	/* get end time and report time taken*/
	gettimeofday(&sthread_end_time,NULL);
	time_t cpu_time_used2 = sthread_end_time.tv_sec - sthread_start_time.tv_sec;
	//printf("start :%f, end: %f\n", (double)start2, (double)end2);
	printf("parallel quick sort took %ld seconds to execute \n", cpu_time_used2);
	printf("your parallel quick sort speed up %f fold!\n",(double)cpu_time_used1/(double)cpu_time_used2 );

	compareArr(copy_arr, arr, arr_size);

	/* free dynamicly allocated memory */
	free(copy_arr);
	free(arr);
}

void randomize(int* arr, int size) {
	//srand(time(0));
	for (int i = 0; i < size; i++) {
		arr[i] = rand() % max_value;
	}
}

void printArr(int* arr, int size) {
	for (int i = 0; i < size; ++i) {
		printf("Element %d: %d\n", i, arr[i]);
	}
}



int* copyArr(int* arr, int size)
{
	int* copy = malloc(sizeof(int) * size);
	for (int i = 0; i < size; i++)
		copy[i] = arr[i];
	return copy;
}

void compareArr(int* correct_arr, int* your_arr, int size)
{
	int wrong_count = 0;
	for (int i = 0; i < size; i++) {
		if (correct_arr[i] == your_arr[i]) {
			//printf("element %d, correct num: %d, your num: %d\n", i, correct_arr[i], your_arr[i]);
		}
		else
		{
			wrong_count++;
			//printf("element %d, correct num: %d, but your num is: %d\n", i, correct_arr[i], your_arr[i]);
		}
	}

	if (wrong_count == 0) {
		printf(" your result is 100 percent correct!\n");
	}
	else {
		printf(" your result has %d wrong nums!", wrong_count);
	}
}






