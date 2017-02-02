#include <omp.h>

void compute(unsigned long **a, unsigned long **b, unsigned long **c, unsigned long **d, int N, int num_threads) {

	omp_set_num_threads(num_threads);
	#pragma omp parallel for 
	for (int i = 1; i < N; i++)
	{
		a[i][1] = 2 * b[i][1];
		d[i][1] = a[i][1] * c[i][1];

		for (int j = 2; j < N; j++)
		{
			a[i][j] = 2 * b[i][j];
			d[i][j] = a[i][j] * c[i][j];
			c[i][j - 2] = a[i][j - 2] - a[i][j];
		}

		c[i][N - 2] = a[i][N - 2] - a[i][N];
	}
}
