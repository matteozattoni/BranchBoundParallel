#ifndef BRANCHBOUNDMEMORYMANAGER_H
#define BRANCHBOUNDMEMORYMANAGER_H

#include "algorithm/branchboundproblem.h"

class BranchBoundMemoryManager
{
private:
    /* data */
public:
    virtual BranchBoundProblem* getBranchBoundProblem() =0;
    BranchBoundMemoryManager(/* args */) {};
    virtual ~BranchBoundMemoryManager() {};
};
#endif