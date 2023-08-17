#include <iostream>
#include <list>
#include <algorithm>
#include <chrono>
#include "knapsack/knapsackmemorymanager.h"
#include "knapsack/knapsack.h"
#include "branchbound/branchbound.h"
#include "branchbound/mpi/mpimanager.h"
#include "mpi.h"

using namespace std;

#define SMALL_DATASET1 "datasets/small/f1_l-d_kp_10_269"      // opt 295
#define SMALL_DATASET2 "datasets/small/f2_l-d_kp_20_878"       // opt 1024
#define SMALL_DATASET3 "datasets/small/f3_l-d_kp_4_20"       // opt 35
#define SMALL_DATASET4 "datasets/small/f4_l-d_kp_4_11"       // opt 23
#define SMALL_DATASET5 "datasets/small/f6_l-d_kp_10_60"       // opt 52
#define SMALL_DATASET6 "datasets/small/f7_l-d_kp_7_50"       // opt 107
#define SMALL_DATASET7 "datasets/small/f8_l-d_kp_23_10000"       // opt 9767
#define SMALL_DATASET8 "datasets/small/f9_l-d_kp_5_80"       // opt 130
#define SMALL_DATASET9 "datasets/small/f10_l-d_kp_20_879"       // opt 1025


#define MEDIUM_DATASET "datasets/knapPI_1_100_1000_1" // opt 9147
#define MEDIUM_DATASET2 "datasets/knapPI_1_200_1000_1" // opt 11238
#define LARGE_DATASET "datasets/knapPI_1_10000_1000_1" // optimus is 563647

struct Data {
    void* ptr;
    double value1;
    double value2;
};

struct Data2 {
    void* ptr;
    int value1;
    bool value2;
};

void test2() {
    
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype mpi_data_type;

    MPI_Aint offsets[2];
    MPI_Aint a,b, base;
    Data2 test;
    MPI_Get_address(&test, &base);
    MPI_Get_address(&test.value1, &a);
    MPI_Get_address(&test.value2, &b);
    offsets[0] = a - base;
    offsets[1] = b - base;
    //offsets[0] = offsetof(Data2, value1);
    //offsets[1] = offsetof(Data2, value2);

    // Definisci un tipo derivato personalizzato che inizia dal doppio offset
    MPI_Type_create_struct(2,                 // Numero di blocchi
                           new int[2]{1, 1}, // Numero di elementi in ogni blocco
                           offsets, // Offset
                           new MPI_Datatype[2]{MPI_INT, MPI_C_BOOL}, // Tipi di dati in ogni blocco
                           &mpi_data_type); // Tipo derivato risultante




    MPI_Aint lb, extent;
    MPI_Type_get_extent(mpi_data_type, &lb, &extent);
    
    MPI_Datatype resized_mpi_data_type;
    MPI_Type_create_resized(mpi_data_type, lb, sizeof(Data2), &resized_mpi_data_type);

    MPI_Type_commit(&mpi_data_type);
    MPI_Type_commit(&resized_mpi_data_type);

    if (rank == 0) {
        Data2 dataToSend[4];
        dataToSend[0].value1 = 3;
        dataToSend[0].value2 = true;
        dataToSend[1].value1 = 1;
        dataToSend[1].value2 = false;
        dataToSend[2].value1 = 5;
        dataToSend[2].value2 = true;
        dataToSend[3].value1 = 67;
        dataToSend[3].value2 = true;

        MPI_Send(dataToSend, 4, resized_mpi_data_type, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        Data2 receivedData[4];

        MPI_Recv(receivedData, 4, resized_mpi_data_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < 4; ++i) {
            std::cout << "Received values " << i + 1 << ": " << receivedData[i].value1 << " and " << (receivedData[i].value2 == true ? "true" : "false") << std::endl;
        }
    }

    MPI_Type_free(&mpi_data_type);
    MPI_Finalize();
}

void test() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype mpi_data_type;

    MPI_Aint offsets[2];
    offsets[0] = offsetof(Data, value1);
    offsets[1] = offsetof(Data, value2);

    // Definisci un tipo derivato personalizzato che inizia dal doppio offset
    MPI_Type_create_struct(2,                 // Numero di blocchi
                           new int[2]{1, 1}, // Numero di elementi in ogni blocco
                           offsets, // Offset
                           new MPI_Datatype[2]{MPI_DOUBLE, MPI_DOUBLE}, // Tipi di dati in ogni blocco
                           &mpi_data_type); // Tipo derivato risultante




    MPI_Aint lb, extent;
    MPI_Type_get_extent(mpi_data_type, &lb, &extent);
    
    MPI_Datatype resized_mpi_data_type;
    MPI_Type_create_resized(mpi_data_type, lb, sizeof(Data), &resized_mpi_data_type);

    MPI_Type_commit(&mpi_data_type);
    MPI_Type_commit(&resized_mpi_data_type);

    if (rank == 0) {
        Data dataToSend[4];
        dataToSend[0].value1 = 3.14;
        dataToSend[0].value2 = 2.71;
        dataToSend[1].value1 = 1.23;
        dataToSend[1].value2 = 4.56;
        dataToSend[2].value1 = 1.99;
        dataToSend[2].value2 = 4.99;
        dataToSend[3].value1 = 67.22;
        dataToSend[3].value2 = 11.22;

        MPI_Send(dataToSend, 4, resized_mpi_data_type, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        Data receivedData[2];

        MPI_Recv(receivedData, 4, resized_mpi_data_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < 4; ++i) {
            std::cout << "Received values " << i + 1 << ": " << receivedData[i].value1 << " and " << receivedData[i].value2 << std::endl;
        }
    }

    MPI_Type_free(&mpi_data_type);
    MPI_Finalize();
}

int main()
{
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::seconds sec;
    typedef std::chrono::duration<float> fsec;

    //test2();
    //return 0;
    
    Knapsack *knapsack = new Knapsack();
    KnapsackMemoryManager *man = new KnapsackMemoryManager();
    man->testProblemMemory();
    man->testBranchMemory();
    man->testBoundMemory();
    MPIManager* mpiManger = new MPIManager(man);
    BranchBound *branchBound = new BranchBound(mpiManger, knapsack);
    try
    {
        branchBound->start();
        delete branchBound;
        delete knapsack;
        delete man;
        delete mpiManger;
        cout << "Final solution is " << branchBound->bound << endl;
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    } catch(int a) {
        cout << "error: " << a << endl;
        return 1;
    }
    
    
    
    
    //cout << "size " << sizeof(t2) << endl;
    
    /* auto t0 = Time::now();
    Knapsack *knapsack = new Knapsack();
    BranchBound *branchBound = new BranchBound(knapsack);
    KnapsackProblem *problem = KnapsackProblem::problemFromFile(SMALL_DATASET1);
    knapsack->printMemoryInfo(cout);
    cout << "Start Branch and Bound (Knapsack Problem)" << endl;
    try
    {
        branchBound->start(problem, true);
        cout << *branchBound << endl;
        cout << "Final solution is " << branchBound->bound << endl;
        auto t1 = Time::now();
        fsec fs = t1 - t0;
        sec d = std::chrono::duration_cast<sec>(fs);
        cout << "Total duration: " << d.count() << "s" << endl;
        return 0;
    }
    catch (eBranchBoundException e)
    {
        cout << "Exception n: " << e << endl;
        return 1;
    } */
}