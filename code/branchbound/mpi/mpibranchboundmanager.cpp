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
    if (worldRank == 0)
        return dataManager.getRootBranch();
    else
        return nullptr;
}

BranchBoundProblem *MPIBranchBoundManager::getBranchProblem()
{
    MPI_Datatype problemType = dataManager.getProblemType();
    MPI_Datatype problemElementType = dataManager.getProblemElementType();
    void *problemDescritpionBuffer;
    std::pair<void *, int> problemElementsPair;
    if (worldRank == GLOBAL_MASTER_RANK)
    {
        BranchBoundProblem *localProblem = dataManager.getLocalProblem();
        problemDescritpionBuffer = dataManager.getProblemTypeBuffFrom(localProblem);
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getProblemElementBuffFrom(localProblem);
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        return localProblem; // could be memory leak
    }
    else
    {
        problemDescritpionBuffer = dataManager.getEmptyProblemTypeBuff();
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getEmptyProblemElementBuffFromType(problemDescritpionBuffer);
        MPI_Status status;
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        BranchBoundProblem *remoteProblem = dataManager.getRemoteProblem(problemDescritpionBuffer, problemElementsPair);
        return remoteProblem; // could be memory leak
    }
}

void MPIBranchBoundManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    masterpoolManager->prologue(callback);
    workpoolManager->prologue(callback);
}

void MPIBranchBoundManager::epilogue(std::function<const Branch *()> callback)
{
    masterpoolManager->epilogue(callback);
    workpoolManager->epilogue(callback);
}

void MPIBranchBoundManager::sendBound(BranchBoundResultSolution *bound)
{
    masterpoolManager->sendBound(bound);
    workpoolManager->sendBound(bound);
}

BranchBoundResultBranch *MPIBranchBoundManager::getBranch()
{
    if (masterpoolManager->isCommEnabled())
    {
        try
        {
            return workpoolManager->getBranch();
        }
        catch (const MPILocalTerminationException &e)
        {
            return masterpoolManager->getBranch();
        }
    }
    else
    {
        return workpoolManager->getBranch();
    }
}

BranchBoundResultBranch *MPIBranchBoundManager::waitForBranch()
{
    try
    {
        return getBranch();
    }
    catch (const MPILocalTerminationException &e) // Workpool and Masterpool reached Local Termination
    {
        try
        {
            return getBranchOrGlobalTermination();
        }
        catch (const MPIGlobalTerminationException &e) // Workpool and Masterpool reached Global Termination
        {
            double finalbound = 0.0;
            double maxbound = std::max(workpoolManager->getBound(), masterpoolManager->getBound());
            MPI_Reduce(&maxbound, &finalbound, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            throw MPIBranchBoundTerminationException(finalbound);
        }
    }
    throw MPIGeneralException("MPIBranchBoundManager::waitForBranch");
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

bool MPIBranchBoundManager::isCommEnabled()
{
    return true;
}

void MPIBranchBoundManager::broadcastTerminationWithValue(bool value) {}

BranchBoundResultBranch *MPIBranchBoundManager::getBranchOrGlobalTermination()
{
    if (masterpoolManager->isCommEnabled())
    {
        try
        {
            return workpoolManager->waitForBranch();
        }
        catch (const MPIGlobalTerminationException &e) // Workpool reach global termination
        {
            return masterpoolManager->waitForBranch();
        }
    }
    else
    {
        return workpoolManager->waitForBranch();
    }
}

double MPIBranchBoundManager::getBound() {
    return bound;
}