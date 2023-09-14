#ifndef BRANCHBOUND_H
#define BRANCHBOUND_H

#include <set>
#include <queue>
#include <vector>
#include <ostream>

#include "algorithm/branchboundalgorithm.h"
#include "algorithm/branchboundproblem.h"
#include "algorithm/branchboundresult.h"
#include "algorithm/parallelmanager.h"

class BranchBound
{
private:
    struct cmp
    {
        bool operator()(const Branch *x, const Branch *y) const { return x->getNumberOfElements() > y->getNumberOfElements(); }
    };
    // memory manager?
    std::set<Branch*, cmp> branchSet;
    // Algorithm
    // long computation = 0;
    BranchBoundAlgorithm* algorithm;
    ParallelManager* parallelManager;
    //int worldRank;
    bool firstExecution = true;
   
    void addBranchToQueue(Branch* branch);
public:
    // method to loop
    static int rank;
    double bound;
    void start();
    void newBranchBoundResult(BranchBoundResult* result);
    Branch* getTaskFromQueue();
    BranchBound(ParallelManager*, BranchBoundAlgorithm*);
    void setBound(double bound);
    void sendBound(BranchBoundResultSolution* boundToSend);
    friend std::ostream& operator<<(std::ostream& out, const BranchBound& data);
    ~BranchBound();
};

#endif