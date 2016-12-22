#include "DllApi.h"

int DllSum(int param)
{
	int sum = 0;

	if (param > 0)
	{
		for (int i = 1; i <= 1; i++)
		{
			sum += i;
		}
	}

	return sum;
}

int DllSum1()
{
	return DllSum(1);
}

int DllSum2()
{
	return DllSum(2);
}

int DllSum3()
{
	return DllSum(3);
}

int DllSum4()
{
	return DllSum(4);
}