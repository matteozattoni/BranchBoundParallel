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
    std::list<const Branch*> list; // list of task
    // Algorithm
    BranchBoundAlgorithm* algorithm;
    MPIBranchBoundManager* mpiManager;
    const int worldRank;
    bool firstExecution = true;
   
    void addBranchToQueue(const Branch* branch);
public:
    // method to loop
    static int rank;
    int bound;
    void start();
    void newBranchBoundResult(BranchBoundResult* result);
    const Branch* getTaskFromQueue();
    BranchBound(MPIBranchBoundManager*, BranchBoundAlgorithm*);
    void setBound(int bound);
    void sendBound(BranchBoundResultSolution* boundToSend);
    friend std::ostream& operator<<(std::ostream& out, const BranchBound& data);
    ~BranchBound();
};

#endif