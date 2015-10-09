#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "histogram.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

struct pthread_args
{
	int start, stop;
	block_t *blocks;
};

void *get_histogram_part(void *ptr)
{
	// Initialization
	unsigned int i, j, start, stop;
	struct pthread_args *args = ptr;
	
	block_t *blocks = args->blocks;
	start = args->start;
	stop = args->stop;

	// Allocate local histogram
	unsigned int *local_histogram = calloc(NALPHABET, sizeof(unsigned int));
 
	// Build histogram
	for (i = start; i < stop; i++)
	{
		for (j=0; j<BLOCKSIZE; j++)
		{
			if (blocks[i][j] >= 'a' && blocks[i][j] <= 'z')
				local_histogram[blocks[i][j]-'a']++;
			else if(blocks[i][j] >= 'A' && blocks[i][j] <= 'Z')
				local_histogram[blocks[i][j]-'A']++;
		}
	}

	return local_histogram;
}

void get_histogram(unsigned int nBlocks,
		   		   block_t *blocks,
				   histogram_t histogram,
				   unsigned int num_threads)
{

	pthread_t *thread;
	struct pthread_args *thread_arg;
	void *ret_hist;
	unsigned int i, j, k, quo, rem;

	thread = calloc(num_threads, sizeof(*thread));
	thread_arg = calloc(num_threads, sizeof(*thread_arg));

	quo = nBlocks / num_threads;
	rem = nBlocks % num_threads;

	for (i = 0; i < num_threads; i++)
	{
		thread_arg[i].start = i * quo  + MIN(i, rem);
		thread_arg[i].stop = (i + 1) * quo  + MIN(i + 1, rem);
		thread_arg[i].blocks = blocks;
		pthread_create(thread + i, NULL, &get_histogram_part, thread_arg + i);
	}

	for (j = 0; j < num_threads; j++)
	{
		pthread_join(thread[j], &ret_hist);
		for (k = 0; k < NALPHABET; k++)
			histogram[k] += ((unsigned int *)ret_hist)[k];

		free(ret_hist);
	}

	free(thread);
	free(thread_arg);
}
