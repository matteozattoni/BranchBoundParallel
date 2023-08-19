#ifndef MPIBRANCHBOUNDMANAGER_H
#define MPIBRANCHBOUNDMANAGER_H

#include <functional>
#include <vector>

#include "mpidatamanager.h"
#include "workpoolmanager.h"
#include "masterpoolmanager.h"

#define MASTER_RANK 0

class MPIBranchBoundManager: public MPIManager
{
private:
    MPIDataManager &dataManager;
    int worldSize;
    int worldRank;
    MPI_Comm workpoolComm;
    int workpoolRank;
    int workpoolSize;
    MPI_Group masterpoolGroup;
    MPIManager *workpoolManager;
    MPIManager *masterpoolManager;


public:
    MPIBranchBoundManager(MPIDataManager &manager);
    ~MPIBranchBoundManager();
    int getWorldRank() { return worldRank;}
    BranchBoundProblem* getBranchProblem() override;
    BranchBoundResultBranch* waitForBranch() override;
    const Branch* getRootBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    void init();
    void defineType();
    
};


#endif