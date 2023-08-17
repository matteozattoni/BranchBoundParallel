#ifndef MPIMANAGER_H
#define MPIMANAGER_H

#include <functional>
#include <vector>

#include "mpidatamanager.h"
#include "../algorithm/branchboundproblem.h"
#include "../algorithm/branchboundresult.h"

#define MASTER_RANK 0
#define WORKPOOL_N 4

class MPIManager
{
private:
    MPIDataManager &dataManager;
    int worldSize;
    int worldRank;
    MPI_Comm workpoolComm;
    int workpoolRank;
    int workpoolSize;
    MPI_Group masterpoolGroup;
    /* data */
    const struct {
        int bound = 0;
        int branch = 1;
    } tagMessage;
    struct {
        MPI_Request request;
        void* boundBuffer;
    } receiveBound;
    struct {
        void* branchBuffer;
        MPI_Request request;
        bool ready = false;
        int count;
    } receiveBranch[WORKPOOL_N];
    struct {
        std::pair<void*,int> sentBufferAndCount;
        MPI_Request request;
        bool isSent = false;
    } sentBranch[WORKPOOL_N];
    void loadBalance(std::function<void(BranchBoundResult*)>);

public:
    MPIManager(MPIDataManager* manager);
    ~MPIManager();
    int getWorldRank() { return worldRank;}
    BranchBoundProblem* getBranchProblem();
    BranchBoundResultBranch* waitForBranch();
    const Branch* getRootBranch();
    void prologue(std::function<void(BranchBoundResult*)>);
    void epilogue(std::function<const Branch*()>);
    void init();
    void defineType();
    
};


#endif