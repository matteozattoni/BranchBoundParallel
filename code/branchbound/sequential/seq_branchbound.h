#ifndef SEQUENTIAL_BRANCHBOUND_H
#define SEQUENTIAL_BRANCHBOUND_H

#include "../algorithm/branchboundalgorithm.h"
#include "../algorithm/branchboundproblem.h"
#include "../algorithm/branchboundresult.h"

#include <list>

class SequentialBranchBound
{
private:
    int bound;
    std::list<Branch*> list;
    BranchBoundAlgorithm* algorithm;
    /* data */
public:
    SequentialBranchBound(BranchBoundAlgorithm *algorithm);
    ~SequentialBranchBound();
    void start(BranchBoundProblem *problem, Branch *rootBranch);
    void setBound(int bound);
    int getBound();
    void newBranchBoundResult(BranchBoundResult* result);
};




#endif