#include <iostream>
#include <list>
#include <algorithm>
#include <chrono>
#include "knapsack/knapsackmemorymanager.h"
#include "knapsack/knapsack.h"
#include "branchbound/branchbound.h"
#include "branchbound/sequential/seq_branchbound.h"
#include "branchbound/mpi/mpibranchboundmanager.h"
#include "mpi.h"

using namespace std;

struct Data
{
    void *ptr;
    double value1;
    double value2;
};

struct Data2
{
    void *ptr;
    int value1;
    bool value2;
};

struct Data3
{
    void *ptr1;
    int value1;
    void *ptr2, *ptr3;
};

void test3()
{

    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype mpi_data_type;

    MPI_Aint offsets[1];
    MPI_Aint a, base;
    Data3 test;
    MPI_Get_address(&test, &base);
    MPI_Get_address(&test.value1, &a);
    offsets[0] = a - base;
    // offsets[0] = offsetof(Data2, value1);
    // offsets[1] = offsetof(Data2, value2);

    // Definisci un tipo derivato personalizzato che inizia dal doppio offset
    int numElementBlock[1] = {1};
    MPI_Datatype datatypes[1] = {MPI_INT};
    MPI_Type_create_struct(1,                            // Numero di blocchi
                           numElementBlock,                // Numero di elementi in ogni blocco
                           offsets,                      // Offset
                           datatypes, // Tipi di dati in ogni blocco
                           &mpi_data_type);              // Tipo derivato risultante

    MPI_Aint lb, extent;
    MPI_Type_get_extent(mpi_data_type, &lb, &extent);

    MPI_Datatype resized_mpi_data_type;
    MPI_Type_create_resized(mpi_data_type, lb, sizeof(Data3), &resized_mpi_data_type);

    MPI_Type_commit(&mpi_data_type);
    MPI_Type_commit(&resized_mpi_data_type);

    if (rank == 0)
    {
        Data3 dataToSend[4];
        dataToSend[0].value1 = 3;
        dataToSend[1].value1 = 1;
        dataToSend[2].value1 = 5;
        dataToSend[3].value1 = 67;

        MPI_Send(dataToSend, 4, resized_mpi_data_type, 1, 0, MPI_COMM_WORLD);
    }
    else if (rank == 1)
    {
        Data3 receivedData[4];

        MPI_Recv(receivedData, 4, resized_mpi_data_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < 4; ++i)
        {
            std::cout << "Received values " << i + 1 << ": " << receivedData[i].value1 << std::endl;
        }
    }

    MPI_Type_free(&mpi_data_type);
    MPI_Finalize();
}

void test2()
{

    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype mpi_data_type;

    MPI_Aint offsets[2];
    MPI_Aint a, b, base;
    Data2 test;
    MPI_Get_address(&test, &base);
    MPI_Get_address(&test.value1, &a);
    MPI_Get_address(&test.value2, &b);
    offsets[0] = a - base;
    offsets[1] = b - base;
    // offsets[0] = offsetof(Data2, value1);
    // offsets[1] = offsetof(Data2, value2);

    // Definisci un tipo derivato personalizzato che inizia dal doppio offset
    int numElementsBlock[2] = {1,1};
    MPI_Datatype datatypes[2] = {MPI_INT, MPI_C_BOOL};
    MPI_Type_create_struct(2,                                        // Numero di blocchi
                           numElementsBlock,                         // Numero di elementi in ogni blocco
                           offsets,                                  // Offset
                           datatypes, // Tipi di dati in ogni blocco
                           &mpi_data_type);                          // Tipo derivato risultante

    MPI_Aint lb, extent;
    MPI_Type_get_extent(mpi_data_type, &lb, &extent);

    MPI_Datatype resized_mpi_data_type;
    MPI_Type_create_resized(mpi_data_type, lb, sizeof(Data2), &resized_mpi_data_type);

    MPI_Type_commit(&mpi_data_type);
    MPI_Type_commit(&resized_mpi_data_type);

    if (rank == 0)
    {
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
    }
    else if (rank == 1)
    {
        Data2 receivedData[4];

        MPI_Recv(receivedData, 4, resized_mpi_data_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < 4; ++i)
        {
            std::cout << "Received values " << i + 1 << ": " << receivedData[i].value1 << " and " << (receivedData[i].value2 == true ? "true" : "false") << std::endl;
        }
    }

    MPI_Type_free(&mpi_data_type);
    MPI_Finalize();
}

void test()
{
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Datatype mpi_data_type;

    MPI_Aint offsets[2];
    offsets[0] = offsetof(Data, value1);
    offsets[1] = offsetof(Data, value2);

    // Definisci un tipo derivato personalizzato che inizia dal doppio offset
    int numElementsBlock[2] = {1,1};
    MPI_Datatype datatypes[2] = {MPI_DOUBLE, MPI_DOUBLE};
    MPI_Type_create_struct(2,                                           // Numero di blocchi
                           numElementsBlock,                            // Numero di elementi in ogni blocco
                           offsets,                                     // Offset
                           datatypes, // Tipi di dati in ogni blocco
                           &mpi_data_type);                             // Tipo derivato risultante

    MPI_Aint lb, extent;
    MPI_Type_get_extent(mpi_data_type, &lb, &extent);

    MPI_Datatype resized_mpi_data_type;
    MPI_Type_create_resized(mpi_data_type, lb, sizeof(Data), &resized_mpi_data_type);

    MPI_Type_commit(&mpi_data_type);
    MPI_Type_commit(&resized_mpi_data_type);

    if (rank == 0)
    {
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
    }
    else if (rank == 1)
    {
        Data receivedData[2];

        MPI_Recv(receivedData, 4, resized_mpi_data_type, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < 4; ++i)
        {
            std::cout << "Received values " << i + 1 << ": " << receivedData[i].value1 << " and " << receivedData[i].value2 << std::endl;
        }
    }

    MPI_Type_free(&mpi_data_type);
    MPI_Finalize();
}

void runKnapsackSequential() {
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::seconds sec;
    typedef std::chrono::duration<float> fsec;

    auto t0 = Time::now();
    KnapsackMemoryManager *manSeq = new KnapsackMemoryManager();

    Knapsack *knapsackSeq = new Knapsack();
    BranchBoundProblem *problemSeq = manSeq->getLocalProblem();
    Branch *rootBranch = manSeq->getRootBranch();
    SequentialBranchBound seqBranchBound(knapsackSeq);

    cout << "Start Branch & Bound Serial" << endl;
    seqBranchBound.start(problemSeq, rootBranch);
    cout << "Final solution is " << seqBranchBound.getBound() << endl;
    delete manSeq;
    //delete knapsackSeq;
    delete problemSeq;
    
    auto t1 = Time::now();
    fsec fs = t1 - t0;
    sec d = std::chrono::duration_cast<sec>(fs);
    cout << "Total duration: " << d.count() << "s" << endl;
}

int runKnaspackParallel() {
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::seconds sec;
    typedef std::chrono::duration<float> fsec;

    auto t0 = Time::now();
    Knapsack *knapsack = new Knapsack();
    KnapsackMemoryManager *man = new KnapsackMemoryManager();
    man->testProblemMemory();
    man->testBranchMemory();
    man->testBoundMemory();
    MPIBranchBoundManager *mpiManger = new MPIBranchBoundManager(*man);
 
    BranchBound *branchBound = new BranchBound(mpiManger, knapsack);
    try
    {
        branchBound->start();
        delete branchBound;
        //delete knapsack;
        delete man;
        delete mpiManger;
        return 1;
    }
    catch (MPIBranchBoundTerminationException &e)
    {   
        cout << "Final solution is " << e.finalSolution << endl;
        delete branchBound;
        //delete knapsack;
        delete man;
        delete mpiManger;
        auto t1 = Time::now();
        fsec fs = t1 - t0;
        sec d = std::chrono::duration_cast<sec>(fs);
        cout << "Total duration: " << d.count() << "s" << endl;
        //knapsack->printAlgorithm(cout);
        return 0;
    }
    catch (int e)
    {
        if (e == 0)
        {
            //knapsack->printAlgorithm(cout);
            delete branchBound;
            //delete knapsack;
            delete man;
            delete mpiManger;
            return 0;
        }

        cout << "error " << e << endl;
        // std::cerr << e << '\n';
        return 1;
    }
}

int main()
{    
    // test3();
    //runKnapsackSequential();

    return runKnaspackParallel();
    return 0;
    
}