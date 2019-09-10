#include "func.h"

int fact(int n)
{
	int ret = 1;
	while (n > 1)
		ret *= n--;
	return ret;
}

int static_val()
{
	static int count = 1;
	return ++count;

}

int mabs(int n)
{
	return (n > 0) ? n : -n;
}