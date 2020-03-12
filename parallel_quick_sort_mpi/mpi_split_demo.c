/******************************************************************************
* FILE: mpi_array.c
* DESCRIPTION:
*   MPI Example - Array Assignment - C Version
*   This program demonstrates a simple data decomposition. The master task
*   first initializes an array and then distributes an equal portion that
*   array to the other tasks. After the other tasks receive their portion
*   of the array, they perform an addition operation to each array element.
*   They also maintain a sum for their portion of the array. The master task
*   does likewise with its portion of the array and any leftover elements.
*   As each of the non-master tasks finish, they send their updated portion
*   of the array to the master.  An MPI collective communication call is used
*   to collect the local sums maintained by each task.  Finally, the master
*   task  displays selected parts of the final array and the global sum of
*   all array elements.
* AUTHOR: Blaise Barney
* LAST REVISED: 07/03/19
****************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define  ARRAYSIZE	20000000
#define  MASTER		0

double  data[ARRAYSIZE];

int main (int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    // Get the rank and size in the original communicator
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int color = world_rank / 4; // Determine color based on row

// Split the communicator based on the color and use the
// original rank for ordering
    MPI_Comm row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);

    int row_rank, row_size;
    MPI_Comm_rank(row_comm, &row_rank);
    MPI_Comm_size(row_comm, &row_size);

    printf("WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE: %d/%d\n",
           world_rank, world_size, row_rank, row_size);

    MPI_Comm_free(&row_comm);
    MPI_Finalize();

}   /* end of main */



