#include "RandomArray.h"





void RandomArray::randomlizeArray(vector<int>& vec)
{
	generate(vec.begin(), vec.end(), generateRandomNum);
}

int RandomArray::generateRandomNum()
{
	return rand() % INT_MAX;
}

void RandomArray::printVec(vector<int>& vec)
{
	for (auto i : vec)
		cout << i <<" ";
	cout << endl;
}

RandomArray::RandomArray()
{
}
