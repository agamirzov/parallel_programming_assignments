#include <stdio.h>
#include <omp.h>

void compute(unsigned long **a, unsigned long **b, unsigned long **c, unsigned long **d, int N, int num_threads)
{
	omp_set_num_threads(num_threads);

	#pragma omp parallel
	{
		for (int i = 1; i < N; i++)
		{
			#pragma omp for nowait
			for (int j = 1; j < N; j++)
			{
				c[i][j] = 3 * d[i][j];
			}
			#pragma omp for nowait
			for (int j = 1; j < N; j++)
			{
				d[i][j]   = 2 * c[i + 1][j];
			}
		}

		#pragma omp barrier

		for (int j = 1; j < N; j++)
		{
			#pragma omp for nowait
			for (int i = 1; i < N; i++)
			{	
				a[i][j]   = a[i][j] * b[i][j];
				b[i][j + 1] = 2 * a[i][j] * c[i - 1][j];
			}
		}
	}
}
