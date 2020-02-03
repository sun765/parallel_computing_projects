#pragma once
#include<vector>

#include<stdlib.h>
#include<algorithm>
#include<thread>
#include<iostream>

using namespace std;
class ThreadedQuickSort
{
public:
	static void thradedQSort(vector<int>& arr, int thread_num);
	static void navieQSort(vector<int>& arr, int thread_num);
	static void qSortHelper(vector<int>& arr, vector<thread>& threads,
		                    int arr_start, int arr_end, 
		                    int thread_start, int thread_end);
};

