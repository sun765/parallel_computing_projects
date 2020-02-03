#pragma once
#include<vector>
#include<stdlib.h>
#include<algorithm>
#include<iostream>

using namespace std;

class RandomArray
{
	public:
		static void randomlizeArray(vector<int>& vec);
		static int generateRandomNum();
		static void printVec(vector<int>& vec);
		RandomArray();
		/*
			how to use functor inside a class?
			static class RandomGenerator {
				int max_val;

				RandomGenerator(int max) : max_val(max) {

				}

				int operator()() {
					return rand() % this->max_val;
				}
			};
		*/

	private:

};

