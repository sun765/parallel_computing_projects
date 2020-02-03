
#include <iostream>
#include <vector>
#include <thread>
#include <ctime>
#include <iomanip> 
#include <chrono>

#include "RandomArray.h"
#include "ThreadedQuickSort.h"

using namespace std::chrono;
//using namespace std;



// how to receive input?
// main thread is an additional thread?
const int input_size = 10;
const int thread_num = 1;


int main()
{
    /*  
		*************************************************************************************
		1.Initializes an array of specified size with random numbers. This is done in serial
	    execution(no threads at this point)
		*************************************************************************************
	*/

		std::vector<int> arr(input_size);
		RandomArray::randomlizeArray(arr);
		RandomArray::printVec(arr);

	/*  
		*************************************************************************************
		2.Starts the execution clock.
		*************************************************************************************
	*/
	
		time_t start_time = system_clock::to_time_t(system_clock::now());
		//convert it to tm struct
		struct tm time;
		localtime_s(&time, &start_time);
		cout << "Begin time: " << put_time(&time, "%X") << '\n';
		std::cout << "Begin threaded quick sort...\n";
		cout << endl;
	/*
		*************************************************************************************
		3.Creates the specied number of threads that execute parallel quicksort.
		*************************************************************************************
	*/

	/*
		*************************************************************************************
		4.Waits for all threads to be done and stops the execution clock.
		*************************************************************************************
	*/
		ThreadedQuickSort::thradedQSort(arr, 10);
		RandomArray::printVec(arr);

	/*
		*************************************************************************************
		5.Reports the execution time.
		*************************************************************************************
	*/

		time_t finish_time = system_clock::to_time_t(system_clock::now());
		//convert it to tm struct
		localtime_s(&time, &finish_time);
		cout << "Finish time: " << put_time(&time, "%X") << '\n';
		std::cout << "Finished threaded quick sort\n";
		std::cout << "Time spent: " << difftime(finish_time, start_time)<<" seconds";

		return 0;
}


