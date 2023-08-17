#ifndef BRANCHBOUND_H
#define BRANCHBOUND_H

#include <list>
#include <ostream>

#include "algorithm/branchboundalgorithm.h"
#include "algorithm/branchboundproblem.h"
#include "algorithm/branchboundresult.h"
#include "branchboundmemorymanager.h"
#include "mpi/mpimanager.h"

class BranchBound
{
private:
    // memory manager?
    std::list<const Branch*> list; // list of task
    // Algorithm
    BranchBoundAlgorithm* algorithm;
    MPIManager* mpiManager;
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
    BranchBound(MPIManager*, BranchBoundAlgorithm*);
    void setBound(int bound);
    friend std::ostream& operator<<(std::ostream& out, const BranchBound& data);
    ~BranchBound();
};

#endif