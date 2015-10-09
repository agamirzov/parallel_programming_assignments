#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "histogram.h"

#define BUFFER_SIZE 3291648

struct pthread_args
{
	pthread_mutex_t *mutex;
	unsigned int *index;
	unsigned int chunk_size;
	unsigned int *histogram;
	char *buffer;
};

void *get_histogram_part(void *ptr)
{
	// Initialization
	struct pthread_args *args = (struct pthread_args*)ptr;
	unsigned int index, chunk_size;
	
	pthread_mutex_t *mutex = args->mutex;
	chunk_size = args->chunk_size;
	char *buffer = args->buffer;

	// Allocate local histogram
	unsigned int *local_histogram = calloc(NALPHABET, sizeof(unsigned int));

	while(true)
	{
		// Define new interval
		pthread_mutex_lock(mutex);
		index = *(args->index);
		// if (buffer[index]==TERMINATOR)
		// {
		// 	pthread_mutex_unlock(mutex);
		// 	break;
		// }
		*(args->index) = index + chunk_size;
		pthread_mutex_unlock(mutex);

		// Break with cheating
		if (index > BUFFER_SIZE)
			break;

		// Add to local histogram for given interval
		for (int i=index; i<(index + chunk_size); i++)
		{
			if (buffer[i] >= 'a' && buffer[i] <= 'z')
				local_histogram[buffer[i]-'a']++;
			else if(buffer[i] >= 'A' && buffer[i] <= 'Z')
				local_histogram[buffer[i]-'A']++;
		}
	}
	
	args->histogram = local_histogram;

	return NULL;
}

void get_histogram(char *buffer,
		   		   unsigned int* histogram,
		   		   unsigned int num_threads,
			  	   unsigned int chunk_size)
{
	// Initialization
	pthread_t *thread;
	struct pthread_args *thread_arg;

	thread = calloc(num_threads, sizeof(*thread));
	thread_arg = calloc(num_threads, sizeof(*thread_arg));

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	unsigned int index = 0;

	// Create threads
	for (int i = 0; i < num_threads; i++)
	{
		thread_arg[i].mutex = &mutex;
		thread_arg[i].index = &index;
		thread_arg[i].chunk_size = chunk_size;
		thread_arg[i].buffer = buffer;
		pthread_create(thread + i, NULL, &get_histogram_part, thread_arg + i);
	}

	// Join threads
	for (int i = 0; i < num_threads; i++)
	{
		pthread_join(thread[i], NULL);
		for (int k = 0; k < NALPHABET; k++)
			histogram[k] += thread_arg[i].histogram[k];

		free(thread_arg[i].histogram);
	}

	free(thread);
	free(thread_arg);
}
