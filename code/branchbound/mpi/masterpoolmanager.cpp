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
    throw MPIUnimplementedException("MPIGlobalManager::getBranchProblem()");
}

const Branch* MasterpoolManager::getRootBranch() {
    throw MPIUnimplementedException("MPIGlobalManager::getRootBranch()");
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