#include "mpibranchboundmanager.h"
#include <iostream>
#include <cmath>

using namespace std;

MPIBranchBoundManager::MPIBranchBoundManager(MPIDataManager &manager) : MPIManager(manager), dataManager(manager)
{
    MPI_Init(NULL, NULL);
    manager.commitDatatypes();
    masterpoolManager = new MasterpoolManager(manager);
    workpoolManager = new WorkpoolManager(manager);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    
}

MPIBranchBoundManager::~MPIBranchBoundManager()
{
    delete masterpoolManager;
    delete workpoolManager;
    MPI_Finalize();
}

const Branch *MPIBranchBoundManager::getRootBranch()
{
    return masterpoolManager->getRootBranch();
}

void MPIBranchBoundManager::init()
{
}

BranchBoundProblem *MPIBranchBoundManager::getBranchProblem()
{
    return masterpoolManager->getBranchProblem();
}

void MPIBranchBoundManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    
    workpoolManager->prologue(callback);

}



void MPIBranchBoundManager::epilogue(std::function<const Branch *()> callback)
{
    workpoolManager->epilogue(callback);
}

void MPIBranchBoundManager::sendBound(BranchBoundResultSolution *bound)
{
    workpoolManager->sendBound(bound);
}

BranchBoundResultBranch *MPIBranchBoundManager::waitForBranch()
{
    try
    {
        return workpoolManager->waitForBranch();
    }
    catch(const MPIWorkpoolTerminationException& e)
    {
        //cout << "workpool terminated" << endl;
        throw MPIGlobalTerminationException();
    }
    throw MPIGeneralException("waitForBranch");
}

void MPIBranchBoundManager::defineType()
{
    struct
    {
        int a;
        double b;
        double c;
    } myss;

    int size;
    MPI_Aint lb, ext;
    MPI_Datatype mystruct;

    int arrayOfBlocklenghs[] = {1, 2};
    MPI_Aint arrayOfDisplacements[2];
    MPI_Datatype arrayOfOldDatatype[] = {MPI_INT, MPI_DOUBLE};
    MPI_Get_address(&myss.a, &arrayOfDisplacements[0]);
    MPI_Get_address(&myss.b, &arrayOfDisplacements[1]);
    arrayOfDisplacements[1] -= arrayOfDisplacements[0];
    cout << "disp is " << arrayOfDisplacements[1] << endl;
    arrayOfDisplacements[0] = 0;
    MPI_Type_create_struct(2, arrayOfBlocklenghs, arrayOfDisplacements, arrayOfOldDatatype, &mystruct);
    MPI_Type_commit(&mystruct);
    MPI_Type_size(mystruct, &size);
    MPI_Type_get_extent(mystruct, &lb, &ext);
    cout << "mpi size is " << size << " lb: " << lb << " ext: " << ext << endl;
    cout << "struct size is " << sizeof(myss) << endl;
}