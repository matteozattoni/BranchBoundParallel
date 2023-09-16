#ifndef OPENMPMANAGER_H
#define OPENMPMANAGER_H

#include <list>
#include "../algorithm/parallelmanager.h"
#include "../algorithm/branchboundalgorithm.h"
#include "../mpi/mpidatamanager.h"

#define NUM_THREADS 1



class OpenMPManager: public ParallelManager
{
private:
    bool globalTermination = false;
    int numOfThreadWaitingForBranch = 0;
    std::list<const Branch*> listOfBranches;
    MPIDataManager &dataManager;
    ParallelManager* nextManager;
    BranchBoundProblem* problem;
    int thread_id;
    /* data */
public:
    OpenMPManager(MPIDataManager &mpiDataManager);
    ~OpenMPManager();
    BranchBoundProblem* getBranchProblem() override;
    Branch* getRootBranch() override;
    /**
     * returns a BranchBoundResultBranch or throw MPILocalTerminationException
    */
    BranchBoundResultBranch* getBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    bool isCommEnabled() override {
        return true;
    };
    void broadcastTerminationWithValue(bool value) override {};
    double getBound() override { return 0.0;};
    void terminate() override {};
    int getIdentity() override {return 0;};

    void start(std::function<BranchBoundAlgorithm*()>);
};


#endif