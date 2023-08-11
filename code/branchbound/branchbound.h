#ifndef BRANCHBOUND_H
#define BRANCHBOUND_H

#include <list>

#include "branchboundalgorithm.h"
#include "branchboundproblem.h"
#include "result.h"

class BranchBound
{
private:
    // memory manager?

    std::list<const Branch*> list; // list of task
    // Algorithm
    BranchBoundAlgorithm* algorithm;

    void computeOneStep();
    const Branch* getTaskFromQueue();
    void addBranchToQueue(const Branch* branch);
public:
    // method to loop
    int bound;
    void start(BranchBoundProblem*,bool);
    BranchBound(BranchBoundAlgorithm*);
    void setBound(int bound);
    ~BranchBound();
};

#endif