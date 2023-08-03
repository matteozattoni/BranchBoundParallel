#include "mpi.h"
#include <stdio.h>

int main() {
    // Default size is 4
    int size;
    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("size is %d\n", size);
    MPI_Finalize();
    return 0;
}