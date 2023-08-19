#include "mpiglobalmanager.h"


MPIGlobalManager::MPIGlobalManager(MPIDataManager &manager): MPIManager(manager)
{
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
}

MPIGlobalManager::~MPIGlobalManager()
{
}

BranchBoundProblem* MPIGlobalManager::getBranchProblem() {
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

const Branch* MPIGlobalManager::getRootBranch() {
    if (worldRank == 0)
        return dataManager.getRootBranch();
    else
        return nullptr;
}


BranchBoundResultBranch* MPIGlobalManager::waitForBranch() {
    throw MPIUnimplementedException("MPIGlobalManager::waitForBranch()");
}


void MPIGlobalManager::prologue(std::function<void(BranchBoundResult*)>) {
    throw MPIUnimplementedException("MPIGlobalManager::prologue");
}


void MPIGlobalManager::epilogue(std::function<const Branch*()>) {
    throw MPIUnimplementedException("MPIGlobalManager::epilogue");
}


void MPIGlobalManager::sendBound(BranchBoundResultSolution* bound) {
    throw MPIUnimplementedException("MPIGlobalManager::sendBound");
}