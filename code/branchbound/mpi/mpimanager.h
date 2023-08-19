#ifndef MPIMANAGER_H
#define MPIMANAGER_H

#include <functional>
#include <vector>

#include "mpidatamanager.h"
#include "../algorithm/branchboundproblem.h"
#include "../algorithm/branchboundresult.h"

#define MASTER_RANK 0
#define WORKPOOL_N 4

enum eTokenColor {tokenWhite, tokenBlack};
enum eNodeColor {nodeWhite , nodeBlack};
enum mpiException { TERMINATED};

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
    enum tagMessage { TAG_BOUND, TAG_BRANCH, TAG_TOKEN, TAG_TERMINATION};
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
    struct {
        eTokenColor tokenColor = tokenWhite;
        eNodeColor nodeColor = nodeBlack;
        bool hasToken = false;
        MPI_Request request;
    } tokenTermination;
    void loadBalance(std::function<void(BranchBoundResult*)>);
    void receiveBoundMessage(std::function<void(BranchBoundResult*)>);
    BranchBoundResultBranch* returnBranchFromStatus(MPI_Status status);
    // return branch or throw terminate
    BranchBoundResultBranch* terminationProtocol();
    void sendToken(eTokenColor sendWithThisColor);

public:
    MPIManager(MPIDataManager* manager);
    ~MPIManager();
    int getWorldRank() { return worldRank;}
    BranchBoundProblem* getBranchProblem();
    BranchBoundResultBranch* waitForBranch();
    const Branch* getRootBranch();
    void prologue(std::function<void(BranchBoundResult*)>);
    void epilogue(std::function<const Branch*()>);
    void sendBound(BranchBoundResultSolution* bound);
    void init();
    void defineType();
    
};


#endif