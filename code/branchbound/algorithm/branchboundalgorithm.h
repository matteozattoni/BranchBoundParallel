#ifndef BRANCHBOUNDALGORITHM_H
#define BRANCHBOUNDALGORITHM_H

#include "branchboundresult.h"
#include "branchboundbranch.h"
#include "branchboundproblem.h"


class BranchBoundAlgorithm
{
protected:
    BranchBoundProblem* problem;
    /* data */
public:
    virtual bool isBetterBound(int bound) = 0;
    virtual void setBound(int bound) = 0;
    virtual void setProblem(BranchBoundProblem*) = 0;
    virtual void setProblemWithRootBranch(BranchBoundProblem*) = 0;
    virtual void setBranch(Branch*) = 0;
    virtual BranchBoundResult* computeTaskIteration() = 0;
    virtual bool hasCurrentBranch() = 0;
    virtual std::ostream& printAlgorithm(std::ostream& out)=0;
    BranchBoundAlgorithm() {}
    ~BranchBoundAlgorithm() {}
};

#endif