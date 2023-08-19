#ifndef MPIGLOBALMANAGER_H
#define MPIGLOBALMANAGER_H

#include "mpimanager.h"
#include "mpiexceptions.h"

#define GLOBAL_MASTER_RANK 0

class MPIGlobalManager: public MPIManager
{
private:
    int worldSize;
    int worldRank;
    /* data */
public:
    BranchBoundProblem* getBranchProblem() override;
    const Branch* getRootBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    MPIGlobalManager(MPIDataManager &manager);
    ~MPIGlobalManager();
};

#endif