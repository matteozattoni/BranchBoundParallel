#ifndef MPIDATAMANAGER_H
#define MPIDATAMANAGER_H

#include "../algorithm/branchboundproblem.h"
#include "../algorithm/branchboundresult.h"
#include <utility>
#include <mpi.h>


class MPIDataManager
{
private:
    /* data */
public:
    MPIDataManager() {};

    // Branch and Bound Datatype
    virtual MPI_Datatype getProblemType()=0;
    virtual MPI_Datatype getProblemElementType()=0;
    virtual MPI_Datatype getBoundType()=0;
    virtual MPI_Datatype getBranchType()=0;
    virtual void commitDatatypes()=0;

    // Branch and Bound Problem
    // master (0)
    virtual BranchBoundProblem* getLocalProblem() const =0;
    virtual void* getProblemTypeBuffFrom(BranchBoundProblem* problem) = 0;
    virtual std::pair<void*,int> getProblemElementBuffFrom(BranchBoundProblem* problem) =0;

    virtual const Branch* getRootBranch()=0;

    // slaves
    virtual void* getEmptyProblemTypeBuff() =0;
    virtual std::pair<void*,int> getEmptyProblemElementBuffFromType(void* problemType)=0;
    virtual BranchBoundProblem* getRemoteProblem(void* problemType, std::pair<void*,int> problemElements) =0;

    // BOUND
    virtual void* getEmptybBoundBuff()=0;
    virtual BranchBoundResultSolution* getSolutionFromBound(void* buff)=0;
    
    // RECV
    virtual void* getEmptyBranchElementBuff(int count)=0;

    virtual BranchBoundResultBranch* getBranchFromBuff(void* buff, int count)=0;
    
    // SEND
    virtual std::pair<void*,int> getBranchBuffer(const Branch* branch)=0;
    virtual void sentFinished(void* buff, int count)=0;
    

    virtual ~MPIDataManager() {};
};


#endif