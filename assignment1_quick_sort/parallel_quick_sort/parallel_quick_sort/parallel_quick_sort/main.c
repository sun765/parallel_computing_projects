

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

#include "parallel_qsort.h"

/* parameters */
const int arr_size = 10000000;
const int thread_num = 12;
const int max_value = 1000;


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
	int* arr;
	arr = malloc(sizeof(int) * arr_size);

	/* check if the arr has been successfully created */
	if (!arr) {
		perror("Error allocating memory");
		abort();
	}
	
	printf("initialize array with %d random integer\n", arr_size);
	randomize(arr, arr_size);
	int* copy_arr = copyArr(arr, arr_size);


	/****************************************/
	/* pass1: sort the array using 1 thread*/
	/****************************************/
	clock_t start, end;
	double cpu_time_used1;
	time_t start_time = time(NULL);
	start = clock();
	char buffer[26];
	printf("begin single quick sort\n*******************************************************\n\n\n");

	/* single thread q sort*/
	parallel_quicksort(copy_arr, arr_size, 1);

	/* get end time and report time taken*/
	time_t end_time = time(NULL);
	end = clock();
	char buffer2[26];
	cpu_time_used1 = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("single thread quick sort took %f seconds to execute \n\n\n\n", cpu_time_used1);

	/*******************************************************/
	/* pass2: sort the array using required num of threads */
	/*******************************************************/
	/* get start time*/
	double cpu_time_used2;
	start_time = time(NULL);
	start = clock();
	char buffer3[26];
	printf("begin parallel quick sort using %d threads\n*******************************************************\n\n\n", thread_num);

	/* threaded q sort*/
	parallel_quicksort(arr, arr_size, thread_num);

	/* get end time and report time taken*/
	end_time = time(NULL);
	end = clock();
	char buffer4[26];
	cpu_time_used2 = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("parallel quick sort took %f seconds to execute \n", cpu_time_used2);
	printf("your parallel quick sort speed up %f fold!\n",cpu_time_used1/cpu_time_used2 );

	compareArr(copy_arr, arr, arr_size);

	/* free dynamicly allocated memory */
	free(copy_arr);
	free(arr);
}

void randomize(int* arr, int size) {
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






