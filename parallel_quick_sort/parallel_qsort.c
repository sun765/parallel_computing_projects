#include "parallel_qsort.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
//pthread_barrier_t barrier;

/* helper struct to part the array */
typedef struct  {
	int* begin;
	int  size;
}Partition;

/* helper struct for a single thread function call */
typedef struct {
	int* arr_part_p;
	int* buffer_start_p;
	int  arr_size;
	int* prefix_sum_small_p;
	int* prefix_sum_large_p;
	int  pivot;
	int  thread_id;
	int  thread_num;
	pthread_barrier_t *barrier_p;
	pthread_mutex_t* lock_p;
	pthread_cond_t* cv_p;
}ThreadArg;

/* helper struct for a single recursive call */
typedef struct {
	int  arr_is_origin;
	int* arr_start_p;
	int* buffer_start_p;
	int  arr_size;
	int  thread_num;
}RecursiveArg;

/* compare function for qsort */
int compare(const void* a, const void* b) {
	//return 1;
	return (*(int*)a - *(int*)b);
}

/* for testing purpose */
void printArr(int* arr_p, int size);

/* helper function to copy an array to target array*/
void copyArray(int* origin_arr_p, int* target_arr_p, int size);

/* helper function to get pivot */
int getPivot(int* arr, int arr_size) {
	// random select 1000 nums and choose the median
	int  count = min(1000, arr_size);
	int* candidate = malloc(sizeof(int) * count);

	for (int i = 0; i < count; i++) {
		candidate[i] = *(arr + (i) * (arr_size / count));
	}
	qsort(candidate, count, sizeof(int), compare);
	return candidate[count / 2];
}

RecursiveArg* getRecursiveArg(int  arr_is_origin, int* arr_start_p, int* buffer_start_p, int  arr_size, int  thread_num);

/* parallel quick sort recursive helper function */
void* parallel_quicksort_recursive(void *args);

/* parallel quicksort in single thread */
void* parallel_quicksort_thread(void *args);

void parallel_quicksort(int* arr_p, int arr_size, int thread_num)
{
	//printArr(arr_p, arr_size);
	/* create buffer array */
	int* buffer_p = malloc(sizeof(int)*arr_size);

	/* recursively sort the arr */
	RecursiveArg* arg_p = getRecursiveArg(1, arr_p, buffer_p, arr_size, thread_num);
	parallel_quicksort_recursive(arg_p);
	
	//printf(" after local arrangement : \n");
	//printArr(arr_p, arr_size);

	//printArr(buffer_p, 20);
	/* free memory */
	free(buffer_p);
	free(arg_p);
}

RecursiveArg* getRecursiveArg(int arr_is_origin, int* arr_start_p, int* buffer_start_p, int arr_size, int thread_num)
{
	RecursiveArg* arg_p = (RecursiveArg*)malloc(sizeof(RecursiveArg));
	arg_p->arr_is_origin = arr_is_origin;
	arg_p->arr_start_p = arr_start_p;
	arg_p->buffer_start_p = buffer_start_p;
	arg_p->arr_size = arr_size;
	arg_p->thread_num = thread_num;
	return arg_p;
}

void* parallel_quicksort_recursive(void *args)
{
	/* release arguments */
	int  arr_is_origin =  ((RecursiveArg*)args)->arr_is_origin;
	int* arr_start_p =    ((RecursiveArg*)args)->arr_start_p;
	int* buffer_start_p = ((RecursiveArg*)args)->buffer_start_p;
	int  arr_size =       ((RecursiveArg*)args)->arr_size;
	int  thread_num =     ((RecursiveArg*)args)->thread_num;

	/* corner cases:  there's only one thread left */
	if (arr_size ==0)
		return NULL;

	if (thread_num == 1) {
		qsort(arr_start_p, arr_size, sizeof(int), compare);

		// check it's origin array or buffer array. if it's buffer array, then you need to write back to origin array
		if (arr_is_origin==0) {
			//printf("copying to origin array!\n");
			copyArray(arr_start_p, buffer_start_p, arr_size);
		}
		return NULL;
	}

	/* pick a pivot */
	int pivot = getPivot(arr_start_p, arr_size);

	/* create threads and data */
	int* prefix_sum_small_p = malloc(sizeof(int) * thread_num);
	int* prefix_sum_large_p = malloc(sizeof(int) * thread_num);

	memset(prefix_sum_small_p, -1, sizeof(int) * thread_num);
	memset(prefix_sum_large_p, -1, sizeof(int) * thread_num);

	pthread_t* threads_p =    malloc(sizeof(pthread_t) * thread_num);
	ThreadArg* thread_args =  malloc(sizeof(ThreadArg) * thread_num);
	pthread_barrier_t barrier;
	pthread_barrier_init(&barrier, NULL, thread_num);

	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);

	pthread_cond_t cv;
	pthread_cond_init(&cv, NULL);

	for (int i = 0; i < thread_num; i++) {
		// compute start pointer of array and size for each thread
		int avg_length = arr_size / thread_num;
		int remain = arr_size % thread_num;
		int offset = i*avg_length + (i < remain ? i : remain);
		int part_size = i < remain ? avg_length + 1 : avg_length;

		// init barrier
		
		thread_args[i].arr_size = part_size;
		thread_args[i].arr_part_p = arr_start_p + offset;
		thread_args[i].buffer_start_p = buffer_start_p ;
		thread_args[i].pivot = pivot;
		thread_args[i].prefix_sum_large_p = prefix_sum_large_p + i;
		thread_args[i].prefix_sum_small_p = prefix_sum_small_p + i;
		thread_args[i].thread_id = i;
		thread_args[i].thread_num = thread_num;
		thread_args[i].barrier_p = &barrier;
		thread_args[i].lock_p = &lock;
		thread_args[i].cv_p = &cv;

		pthread_create(&threads_p[i], NULL, parallel_quicksort_thread, &thread_args[i]);
	}
		
	/* wait for all threads to join */
	for (int i = 0; i < thread_num; i++)
		pthread_join(threads_p[i], NULL);


	/* go to next level of recursive call */
	int left_count = prefix_sum_small_p[thread_num - 1];
	int right_count = arr_size - left_count;
	int left_thread_num = (left_count * thread_num) / arr_size;

	// make sure don't assign 0 thread into next call
	if (left_thread_num == 0) {
		left_thread_num = 1;
	}
	else if(left_thread_num == thread_num)
	{
		left_thread_num = thread_num - 1;
	}

	int right_thread_num = thread_num - left_thread_num;

	// left 
	RecursiveArg* left_arg_p = getRecursiveArg(1 - arr_is_origin, buffer_start_p, arr_start_p, left_count, left_thread_num);
	pthread_t left_thread;
	int left_done = -1;
	pthread_create(&left_thread,NULL, parallel_quicksort_recursive,left_arg_p);

	// right 
	RecursiveArg* right_arg_p = getRecursiveArg(1 - arr_is_origin, buffer_start_p + left_count, arr_start_p + left_count, right_count, right_thread_num);
	parallel_quicksort_recursive(right_arg_p);

	pthread_join(left_thread, NULL);
	/* free dynamic allocated memory */
	free(prefix_sum_small_p);
	free(prefix_sum_large_p);
	free(threads_p);
	free(thread_args);
	free(left_arg_p);
	free(right_arg_p);

	return NULL;
}

void* parallel_quicksort_thread(void* args)
{
	/* release argument */
	int arr_size =                ((ThreadArg*)args)->arr_size;
	int* arr_part_p =             ((ThreadArg*)args)->arr_part_p;
	int* buffer_start_p =         ((ThreadArg*)args)->buffer_start_p;
	int pivot =                   ((ThreadArg*)args)->pivot;
	int* prefix_sum_large_p =     ((ThreadArg*)args)->prefix_sum_large_p;
	int* prefix_sum_small_p =     ((ThreadArg*)args)->prefix_sum_small_p;
	int thread_id =               ((ThreadArg*)args)->thread_id;
	int thread_num =              ((ThreadArg*)args)->thread_num;
	pthread_barrier_t* barrer_p = ((ThreadArg*)args)->barrier_p;
	pthread_mutex_t* lock_p =     ((ThreadArg*)args)->lock_p;
	pthread_cond_t* cv_p    =     ((ThreadArg*)args)->cv_p;

	//printf("thread id: %d, thread_num: %d, pivot: %d, arr_size: %d, barrier_P : %d, lock : %d\n", thread_id, thread_num, pivot, arr_size, *barrer_p, *lock_p);

	/* step 1: local arrangement and count the sum */
	int* left_ptr = arr_part_p;
	int* right_ptr = arr_part_p + arr_size - 1;
	while (left_ptr < right_ptr) {
		while (*left_ptr < pivot && left_ptr < right_ptr)
			left_ptr++;
		while (*right_ptr >= pivot && right_ptr > left_ptr)
			right_ptr--;
		
		// swap two value;
		int temp = *left_ptr;
		*left_ptr = *right_ptr;
		*right_ptr = temp;
	}

	int small_count = left_ptr - arr_part_p;
	int large_count = arr_size - small_count;
	*prefix_sum_small_p = small_count;
	*prefix_sum_large_p = large_count;

	// wait for all threads to finish
	pthread_barrier_wait(barrer_p);



	/* step 2: compute prefix sum. right now I just use the most brute force method */
	
	if (thread_id == 0) {
		for (int i = 1; i < thread_num; i++) {
			*(prefix_sum_small_p+i) += *(prefix_sum_small_p +i-1);
			*(prefix_sum_large_p+i) += *(prefix_sum_large_p +i-1);
		}
	}

	// wait for all threads to finish
	pthread_barrier_wait(barrer_p);
	//printf("thread %d done!, prefix sum is  %d \n", thread_id, *prefix_sum_small_p);
	


	/* step 3: write to buffer */
	// write small part to buffer;
	int offset = thread_id == 0? 0: *(prefix_sum_small_p - 1);
	for (int i = 0; i < small_count; i++) {
		int* ptr = arr_part_p + i;
		int* buffer_ptr = buffer_start_p + offset + i;
		*buffer_ptr = *ptr;
	}

	// write large part to buffer;
	int pivot_index = *(prefix_sum_small_p - thread_id + thread_num-1);

	offset = thread_id ==0? 0: *(prefix_sum_large_p - 1);
	for (int i = 0; i < large_count; i++) {
		int* ptr = left_ptr + i;
		int* buffer_ptr = buffer_start_p + pivot_index + offset + i;
		*buffer_ptr = *ptr;
	}

} 

void copyArray(int* origin_arr_p, int* target_arr_p, int size)
{
	for (int i = 0; i < size; i++) {
		*target_arr_p = *origin_arr_p;
		origin_arr_p++;
		target_arr_p++;
	}
}
