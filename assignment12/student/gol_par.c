#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include "helper.h"

unsigned int gol(unsigned char *grid, unsigned int dim_x, unsigned int dim_y, unsigned int time_steps)
{
	// READ ME! Parallelize this function to work with MPI. It must work even with a single processor.
	// We expect you to use MPI_Scatterv, MPI_Gatherv, and MPI_Sendrecv to achieve this.
	// MPI_Scatterv/Gatherv are checked to equal np times, and MPI_Sendrecv is expected to equal 2 * np * timesteps
	// That is, top+bottom ghost cells * all processors must execute this command * Sendrecv executed every timestep.

	int np, rank, quo, rem;
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Allocate length and displacements array
    quo = (dim_y/np)*dim_x;
    rem = (dim_y%np)*dim_x;
    int *length = (int *) calloc(np, sizeof(int));
    int *disps = (int *) calloc(np, sizeof(int));

    // Fill corresponding arrays
    for(int i = 0; i < np - 1; i++)
    {
        disps[i + 1] = disps[i] + quo;
        length[i] = disps[i + 1] - disps[i];
    }
    length[np - 1] = quo + rem;

    // Grids allocation
	unsigned char *loc_grid_in, *loc_grid_tmp, *loc_grid_out;
    loc_grid_in = (unsigned char *) calloc(sizeof(unsigned char), length[rank] + 2*dim_x);
    loc_grid_tmp = (unsigned char *) calloc(sizeof(unsigned char), length[rank] + 2*dim_x);
	if (loc_grid_tmp == NULL)
		exit(EXIT_FAILURE);

	// Distribute parts of grid to other processors
    MPI_Scatterv(grid, length, disps, MPI_UNSIGNED_CHAR,
    			 loc_grid_in + dim_x, length[rank], MPI_UNSIGNED_CHAR,
    			 0,
    			 MPI_COMM_WORLD);

	loc_grid_out = loc_grid_tmp;
	int loc_dim_y = length[rank]/dim_x;
	int frw = (rank + 1 + np) % np;
	int backw = (rank - 1 + np) % np;

	for (int t = 0; t < time_steps; ++t)
	{
		// Forward sendrecv
		MPI_Sendrecv(loc_grid_in + length[rank], dim_x, MPI_UNSIGNED_CHAR, frw, 1,
					 loc_grid_in, dim_x, MPI_UNSIGNED_CHAR, backw, 1,
    				 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Backward sendrecv
		MPI_Sendrecv(loc_grid_in + dim_x, dim_x, MPI_UNSIGNED_CHAR, backw, 0,
					 loc_grid_in + dim_x + length[rank], dim_x, MPI_UNSIGNED_CHAR, frw, 0,
    				 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		for (int y = 1; y < 1 + loc_dim_y; ++y)
		{
			for (int x = 0; x < dim_x; ++x)
			{
				evolve(loc_grid_in, loc_grid_out, dim_x, loc_dim_y + 2, x, y);
			}
		}
		swap((void**)&loc_grid_in, (void**)&loc_grid_out);
	}

	MPI_Gatherv(loc_grid_in + dim_x, length[rank], MPI_UNSIGNED_CHAR,
   				grid, length, disps, MPI_UNSIGNED_CHAR,
   				0,
   				MPI_COMM_WORLD);

	free(loc_grid_in);
	free(loc_grid_out);
	free(disps);
	free(length);
	
	if (rank == 0)
		return cells_alive(grid, dim_x, dim_y);
	else
		return 0;
}