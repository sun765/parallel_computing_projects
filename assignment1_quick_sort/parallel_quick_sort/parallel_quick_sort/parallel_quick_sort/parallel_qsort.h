#ifndef THSORT_H
#define THSORT_H
#define HAVE_STRUCT_TIMESPEC
#define MAX_QUEUE 65536

#include <pthread.h>

#include "threadpool.h"
#include <stdlib.h>

/* parallel quick sort using specific num of threads */
void parallel_quicksort(int* arr, int arr_size, int thread_num);

#endif