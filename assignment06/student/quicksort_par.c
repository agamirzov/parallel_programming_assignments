#include <omp.h>
#include <stdio.h>
#include <math.h>
#include "quicksort.h"
#include "helper.h"

void _quicksort(int *a, int left, int right, int num_threads, int depth)
{
	if(left < right)
	{
		depth += 1;
		int x = left, y = (left+right)/2, z = right;
		int pivotIdx = (a[x] <= a[y])
			? ((a[y] <= a[z]) ? y : ((a[x] < a[z]) ? z : x))
			: ((a[x] <= a[z]) ? x : ((a[y] < a[z]) ? z : y));

		int pivotVal = a[pivotIdx];
		swap(a + pivotIdx, a + right);

		int swapIdx = left;

		for(int i=left; i < right; i++)
		{
			if(a[i] <= pivotVal)
			{
				swap(a + swapIdx, a + i);
				swapIdx++;
			}
		}
		swap(a + swapIdx, a + right);
		

		_quicksort(a, left, swapIdx - 1, num_threads, depth);
		
		#pragma omp task final(depth >= 3)
		_quicksort(a, swapIdx + 1, right, num_threads, depth);
	}
}

void quicksort(int *a, int left, int right, int num_threads)
{
	omp_set_num_threads(num_threads);
	omp_set_dynamic(0);
	int depth = 0;

	#pragma omp parallel
	{
		#pragma omp single
		_quicksort(a, left, right, num_threads, depth);
	}
}
