#ifndef WORKPOOLMANAGER_H
#define WORKPOOLMANAGER_H

#include "mpiexceptions.h"
#include "../algorithm/parallelmanager.h"
#include "mpidatamanager.h"
#include "mpimessage.h"
#include <vector>

class WorkpoolManager: public ParallelManager
{
private:
    MPIDataManager &dataManager;
    double bound=0.0;
    MPI_Comm workpoolComm;
    int numberWorkpool;
    int workpoolRank;
    int workpoolSize;
    enum tagMessage { TAG_BOUND, TAG_BRANCH, TAG_TOKEN, TAG_TERMINATION};
    enum eTokenColor {tokenWhite, tokenBlack};
    enum eNodeColor {nodeWhite , nodeBlack};
    struct {
        void* branchBuffer;
        int numElement;
        MPI_Request request = MPI_REQUEST_NULL;
    } receiveBranch[WORKPOOL_WORKER];
    struct {
        void* branchBuffer;
        int numElement;
        MPI_Request request = MPI_REQUEST_NULL;
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
    Branch* getBranchFromStatus(MPI_Status status);
    Branch* getBranchFromGeneralStatus(MPI_Status status);
    void receiveBoundMessage(std::function<void(BranchBoundResult *)>);
    void sendToken();
    void loadBalance(std::function<void(BranchBoundResult*)>);
    BranchBoundResultSolution *cacheLastBoundMessage = nullptr;
    
    bool isLocalTerminate();
    void checkTermination();
    /* data */
public:
    long totalSendBranch = 0;
    long totalRecvBranch = 0;
    BranchBoundProblem* getBranchProblem() override;
    Branch* getRootBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    BranchBoundResultBranch* getBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    bool isCommEnabled() override;
    void broadcastTerminationWithValue(bool value) override;
    double getBound() override;
    void terminate() override;
    int getIdentity() override;
    WorkpoolManager(MPIDataManager &manager);
    ~WorkpoolManager();
};


#endif