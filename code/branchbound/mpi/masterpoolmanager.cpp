#include "masterpoolmanager.h"


MasterpoolManager::MasterpoolManager(MPIDataManager &manager): MPIManager(manager)
{
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    MPI_Group commGroup;
    MPI_Comm_group(MPI_COMM_WORLD, &commGroup);

}

MasterpoolManager::~MasterpoolManager()
{
}

BranchBoundProblem* MasterpoolManager::getBranchProblem() {
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

const Branch* MasterpoolManager::getRootBranch() {
    if (worldRank == 0)
        return dataManager.getRootBranch();
    else
        return nullptr;
}


BranchBoundResultBranch* MasterpoolManager::waitForBranch() {
    throw MPIUnimplementedException("MPIGlobalManager::waitForBranch()");
}


void MasterpoolManager::prologue(std::function<void(BranchBoundResult*)>) {
    throw MPIUnimplementedException("MPIGlobalManager::prologue");
}


void MasterpoolManager::epilogue(std::function<const Branch*()>) {
    throw MPIUnimplementedException("MPIGlobalManager::epilogue");
}


void MasterpoolManager::sendBound(BranchBoundResultSolution* bound) {
    throw MPIUnimplementedException("MPIGlobalManager::sendBound");
}