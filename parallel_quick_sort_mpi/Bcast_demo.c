#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank;
    int* buf = malloc(sizeof(int)*4);
    const int root=0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == root) {
        for(int i =0;i< 4;i++)
            buf[i] = i;
    }

    //printf("[%d]: Before Bcast, buf is %d\n", rank, buf);

    /* everyone calls bcast, data is taken from root and ends up in everyone's buf */
    MPI_Bcast(&ran, 4, MPI_INT, root, MPI_COMM_WORLD);
    int MPI_Allgather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                      void *recvbuf, int recvcount, MPI_Datatype recvtype,
                      MPI_Comm comm)

    //printf("[%d]: After Bcast, buf is %d\n", rank, buf);
    for(int i =0;i< 4;i++){
        printf("process %d 's %d's element : %d\n", rank, i, buf[i]);
    }

    MPI_Finalize();
    return 0;
}