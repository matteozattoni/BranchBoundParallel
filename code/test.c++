#include "mpi.h"
#include "knapsack/knapsacklib.h"
#include <stdio.h>
#include <vector>
#include <bits/stdc++.h>

using namespace std;

int main() {
    // Default size is 4
    std::vector<int> myvector;
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // set some content in the vector:
    for (int i=0; i<129; i++) myvector.push_back(i);

    if (rank == 0) {
        printf("comm world size is %d\n", size);
        printf("sizeof(struct knapsackelem) = %zu\n", sizeof(struct KnapsackObject));
        printf("alignof(struct knapsackelem) = %zu\n", alignof(struct KnapsackObject));
        printf("sizeof(struct elemSoloution) = %zu\n", sizeof(struct KnapsackElementSolution));
        printf("alignof(struct elemSoloution) = %zu\n", alignof(struct KnapsackElementSolution));
        printf("vector size: %d\n", myvector.size());
        cout << "capacity of vector 2 = " << myvector.capacity() << endl;
        cout << "max_size of vector 2 = " << myvector.max_size() << endl;
    }
    
    MPI_Finalize();
    return 0;
}