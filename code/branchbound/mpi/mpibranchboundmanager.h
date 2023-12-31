#ifndef MPIBRANCHBOUNDMANAGER_H
#define MPIBRANCHBOUNDMANAGER_H

#include <functional>
#include <vector>
#include <chrono>

#include "mpidatamanager.h"
#include "workpoolmanager.h"
#include "masterpoolmanager.h"

class MPIBranchBoundManager: public ParallelManager
{
private:
    MPIDataManager &dataManager;
    typedef std::chrono::high_resolution_clock Time;
    typedef std::chrono::seconds sec;
    typedef std::chrono::duration<float> fsec;
    double bound = 0.0;
    long totalSendBranch = 0;
    long totalRecvBranch = 0;
    int worldSize;
    int worldRank;
    ParallelManager *masterpoolManager;
    enum tagMessage { BRANCH, BRANCH_REQUEST, BOUND, TOKEN, TERMINATION};
    fsec totalComunicationTime;
public:
    MPIBranchBoundManager(MPIDataManager &manager);
    ~MPIBranchBoundManager();
    int getWorldRank() { return worldRank;}
    BranchBoundProblem* getBranchProblem() override;
    BranchBoundResultBranch* getBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    Branch* getRootBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    bool isCommEnabled() override;
    double getBound() override;
    void broadcastTerminationWithValue(bool value) override;
    void terminate() override;
    int getIdentity() override;
    
};


#endif