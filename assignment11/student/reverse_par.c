#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "helper.h"

void reverse(char *str, int strlen)
{
	// Initialization
	int str_len = strlen;
	int np, rank, quo, rem, tag = 1, recv_idx = 0;

    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status stat;

    // Allocate length and displacements array
    quo = str_len/np;
    rem = str_len%np;
    int *length = (int *) calloc(np, sizeof(int));
    int *disps = (int *) calloc(np, sizeof(int));

    // Fill corresponding arrays
    for(int i = 0; i < np - 1; i++)
    {
        disps[i + 1] = disps[i] + quo;
        length[i] = disps[i + 1] - disps[i];
    }
    length[np - 1] = quo + rem;

    // Allocate local str array
    char *loc_str = (char *) calloc(length[rank], sizeof(char));

    // Distribute parts of str to other processors
    MPI_Scatterv(str, length, disps, MPI_CHAR, loc_str, length[rank], MPI_CHAR, 0, MPI_COMM_WORLD);

    // Call reverse function on local str array
    reverse_str(loc_str, length[rank]);

    // Send local arrays back to the root
    if (rank != 0)
    {
    	printf("len = %d rank = %d\n", length[rank], rank);
    	MPI_Send(loc_str, length[rank], MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    }
    else
    {
    	// Receiving arrays on the root to str
    	for (int i = 1; i < np; i++)
    	{
    		MPI_Recv(str + recv_idx, length[np - i], MPI_CHAR, np - i, tag, MPI_COMM_WORLD, &stat);
    		recv_idx += length[np - i];
    	}

    	// Copy last part 
    	memcpy(str + recv_idx, loc_str, length[rank]);
    }
}
