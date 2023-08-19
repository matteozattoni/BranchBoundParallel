#ifndef WORKPOOLMANAGER_H
#define WORKPOOLMANAGER_H

#include "mpimanager.h"

#define WORKPOOL_WORKER 4

class WorkpoolManager: public MPIManager
{
private:
    MPI_Comm workpoolComm;
    int workpoolRank;
    int workpoolSize;
    enum tagMessage { TAG_BOUND, TAG_BRANCH, TAG_TOKEN, TAG_TERMINATION};
    enum eTokenColor {tokenWhite, tokenBlack};
    enum eNodeColor {nodeWhite , nodeBlack};
    struct {
        void* branchBuffer;
        MPI_Request request;
        bool ready = false;
        int count;
    } receiveBranch[WORKPOOL_WORKER];
    struct {
        std::pair<void*,int> sentBufferAndCount;
        MPI_Request request;
        bool isSent = false;
    } sentBranch[WORKPOOL_WORKER];
    struct {
        eTokenColor tokenColor = tokenWhite;
        eNodeColor nodeColor = nodeBlack;
        bool hasToken = false;
        MPI_Request request;
    } tokenTermination;
    struct {
        void* boundBuffer;
        MPI_Request request;
    } receiveBound;
    BranchBoundResultBranch* waitForBranchOrTerminate();
    BranchBoundResultBranch* returnBranchFromStatus(MPI_Status status);
    void receiveBoundMessage(std::function<void(BranchBoundResult *)>);
    void sendToken();
    void loadBalance(std::function<void(BranchBoundResult*)>);
    /* data */
public:
    BranchBoundProblem* getBranchProblem() override;
    const Branch* getRootBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    WorkpoolManager(MPIDataManager &manager);
    ~WorkpoolManager();
};


#endif