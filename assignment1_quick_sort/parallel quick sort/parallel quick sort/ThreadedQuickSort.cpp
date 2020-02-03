#include "ThreadedQuickSort.h"

int compare(const void* a, const void* b)
{
	return (*(int*)a - *(int*)b);
}

// a helper method to chop arr for different threads
struct Partition {
	int start_index;
	int size;
	Partition(int start_index, int size) {
		this->size = size;
		this->start_index = start_index;
	}
};

// get current thread's coresponding array piece.  this should work with array with any length
Partition* getPartition(int arr_start, int arr_end, int cur_thread, int thread_start, int thread_end) {
	//1. corner cases
	try {
		if (arr_start > arr_end) {
			//cout << "Please make sure the indexes are valid!" << endl;
			throw 1;
		}

		int thread_num = thread_end - thread_start + 1;


		if (thread_num == 0) {
			throw 2;
			//cout << "Please make sure that there is at least one thread!" << endl;
		}

		int arr_length = arr_end - arr_start + 1;

		if (thread_num == 1)
			return new Partition(arr_start, arr_length);

		int avg_part_length = arr_length % thread_num == 0 ? arr_length / thread_num : arr_length / (thread_num - 1);
		int part_length = cur_thread == thread_end ? (arr_length-avg_part_length*(thread_num-1)) : avg_part_length;
		int part_start = arr_start + (cur_thread - thread_start) * avg_part_length;
		return new Partition(part_start, part_length);
	}
	catch(int error_id){
		cout << "number "<<error_id<< " in partition function!" << endl;
	}

}

// each thread calc their own sum 
void threadSum(Partition* p, int* start, int size, int* sum_index , int pivot) {

}

void ThreadedQuickSort::thradedQSort(vector<int>& arr, int thread_num)
{
	vector<thread> threads;
	qSortHelper(arr, threads, 0, arr.size() - 1, 0, thread_num - 1);
}

void ThreadedQuickSort::navieQSort(vector<int>& arr, int thread_num)
{
	qsort(&arr[0], arr.size(), sizeof(int), compare);
}

void ThreadedQuickSort::qSortHelper(vector<int>& arr, vector<thread>& threads, int arr_start, int arr_end, int thread_start, int thread_end)
{
	//1. partition array with threads
	int pivot = arr[arr_start];
	int thread_num = thread_end - thread_start+1;
	vector<Partition*> parts;
	vector<int> prefix_sum(thread_num, 0);
	for (int i = thread_start; i <= thread_end; i++) {
		Partition* p = getPartition(arr_start, arr_end, i,thread_start, thread_end);
		parts.push_back(p);
		
	}
	//2. choose pivot, let all other threads know

	//3. each threads count sum of number that are smaller than pivot

	//4. scan sum (both smaller and bigger?)

	//5. merge (two parts )

	//6. do recursive on left and right
}
