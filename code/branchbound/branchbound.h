#ifndef BRANCHBOUND_H
#define BRANCHBOUND_H

#include <list>
#include <ostream>

#include "algorithm/branchboundalgorithm.h"
#include "algorithm/branchboundproblem.h"
#include "algorithm/branchboundresult.h"
#include "branchboundmemorymanager.h"
#include "mpi/mpibranchboundmanager.h"

class BranchBound
{
private:
    // memory manager?
    std::list<Branch*> list; // list of task
    // Algorithm
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