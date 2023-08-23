#include "mpibranchboundmanager.h"
#include <iostream>
#include <cmath>

using namespace std;

MPIBranchBoundManager::MPIBranchBoundManager(MPIDataManager &manager) : MPIManager(manager), dataManager(manager)
{
    MPI_Init(NULL, NULL);
    manager.commitDatatypes();
    masterpoolManager = new MasterpoolManager(manager);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
}

MPIBranchBoundManager::~MPIBranchBoundManager()
{
    delete masterpoolManager;
    MPI_Finalize();
}

Branch *MPIBranchBoundManager::getRootBranch()
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
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        BranchBoundProblem *remoteProblem = dataManager.getRemoteProblem(problemDescritpionBuffer, problemElementsPair);
        return remoteProblem; // could be memory leak
    }
}

void MPIBranchBoundManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    masterpoolManager->prologue(callback);
}

void MPIBranchBoundManager::epilogue(std::function<const Branch *()> callback)
{
    masterpoolManager->epilogue(callback);
}

void MPIBranchBoundManager::sendBound(BranchBoundResultSolution *bound)
{
    masterpoolManager->sendBound(bound);
}

BranchBoundResultBranch *MPIBranchBoundManager::getBranch()
{
    return masterpoolManager->getBranch();
}

BranchBoundResultBranch *MPIBranchBoundManager::waitForBranch()
{
    try
    {
        return masterpoolManager->waitForBranch();
    }
    catch (const MPIGlobalTerminationException &e) // Workpool and Masterpool reached Local Termination
    {
        std::cout << "(" << worldRank <<" ) TERMINATED" << std::endl;
        terminate();

        double finalbound = 0.0;
        double maxbound = masterpoolManager->getBound();
        MPI_Reduce(&maxbound, &finalbound, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        throw MPIBranchBoundTerminationException(finalbound);
    }
    throw MPIGeneralException("MPIBranchBoundManager::waitForBranch");
}

bool MPIBranchBoundManager::isCommEnabled()
{
    return true;
}

void MPIBranchBoundManager::broadcastTerminationWithValue(bool value) {}

double MPIBranchBoundManager::getBound()
{
    return masterpoolManager->getBound();
}

void MPIBranchBoundManager::terminate() {
    masterpoolManager->terminate();
}