#ifndef MPIMANAGER_H
#define MPIMANAGER_H

#include <functional>
#include "../algorithm/branchboundproblem.h"
#include "../algorithm/branchboundresult.h"

class ParallelManager
{   
public:
    virtual BranchBoundProblem* getBranchProblem() =0;
    virtual Branch* getRootBranch() =0;
    /**
     * returns a BranchBoundResultBranch or throw MPILocalTerminationException
    */
    virtual BranchBoundResultBranch* getBranch() =0;
    virtual BranchBoundResultBranch* waitForBranch() =0;
    virtual void prologue(std::function<void(BranchBoundResult*)>) =0;
    virtual void epilogue(std::function<const Branch*()>) =0;
    virtual void sendBound(BranchBoundResultSolution* bound) =0;
    virtual bool isCommEnabled() =0;
    virtual void broadcastTerminationWithValue(bool value) =0;
    virtual double getBound() =0;
    virtual void terminate()=0;
    virtual int getIdentity()=0;

    ParallelManager() {};
    virtual ~ParallelManager() {};
};

#endif