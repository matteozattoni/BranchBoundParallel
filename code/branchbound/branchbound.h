#ifndef BRANCHBOUND_H
#define BRANCHBOUND_H

#include <list>
#include <queue>
#include <vector>
#include <ostream>

#include "algorithm/branchboundalgorithm.h"
#include "algorithm/branchboundproblem.h"
#include "algorithm/branchboundresult.h"
#include "mpi/mpibranchboundmanager.h"

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
    MPIBranchBoundManager* mpiManager;
    int worldRank;
    bool firstExecution = true;
   
    void addBranchToQueue(Branch* branch);
public:
    // method to loop
    static int rank;
    int bound;
    void start();
    void newBranchBoundResult(BranchBoundResult* result);
    Branch* getTaskFromQueue();
    BranchBound(MPIBranchBoundManager*, BranchBoundAlgorithm*);
    void setBound(int bound);
    void sendBound(BranchBoundResultSolution* boundToSend);
    friend std::ostream& operator<<(std::ostream& out, const BranchBound& data);
    ~BranchBound();
};

#endif