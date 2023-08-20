#ifndef MPIBRANCHBOUNDMANAGER_H
#define MPIBRANCHBOUNDMANAGER_H

#include <functional>
#include <vector>

#include "mpidatamanager.h"
#include "workpoolmanager.h"
#include "masterpoolmanager.h"

class MPIBranchBoundManager: public MPIManager
{
private:
    double bound = 0.0;
    MPIDataManager &dataManager;
    int worldSize;
    int worldRank;
    MPI_Comm workpoolComm;
    int workpoolRank;
    int workpoolSize;
    MPIManager *workpoolManager;
    MPIManager *masterpoolManager;

    BranchBoundResultBranch* getBranchOrGlobalTermination();
public:
    MPIBranchBoundManager(MPIDataManager &manager);
    ~MPIBranchBoundManager();
    int getWorldRank() { return worldRank;}
    BranchBoundProblem* getBranchProblem() override;
    BranchBoundResultBranch* getBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    const Branch* getRootBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    bool isCommEnabled() override;
    double getBound() override;
    void broadcastTerminationWithValue(bool value) override;
    void defineType();
    
};


#endif