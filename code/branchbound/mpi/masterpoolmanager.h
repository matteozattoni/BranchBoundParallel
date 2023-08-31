#ifndef MPIGLOBALMANAGER_H
#define MPIGLOBALMANAGER_H

#include "mpimanager.h"
#include "workpoolmanager.h"
#include "mpiexceptions.h"
#include "mpimessage.h"
#include "tokenringmanager.h"
#include <vector>

#define GLOBAL_MASTER_RANK 0

class MasterpoolManager: public MPIManager
{
private:
    double bound = 0.0;
    int masterpoolSize;
    int masterpoolRank;
    int nextRankToSend;
    int previousRankToReceive;
    MPIManager *workpoolManager;
    MPI_Comm masterpoolComm;
    enum tagMessage { RING_BRANCH, TREE_BRANCH, BOUND, TOKEN, TERMINATION};
    enum eTokenColor {tokenWhite, tokenBlack};
    enum eNodeColor {nodeWhite , nodeBlack};
    struct {
        MPI_Request request = MPI_REQUEST_NULL;
        void* buffer;
        int numElement;
    } branchSent;
    struct {
        MPI_Request request = MPI_REQUEST_NULL;
        void* buffer;
        int numElement;
    } ringBranchReceived;
    struct {
        MPI_Request request;
    } boundSent;
    struct {
        eTokenColor tokenColor = tokenWhite;
        eNodeColor nodeColor = nodeBlack;
        bool hasToken = false;
        MPI_Request request;
    } tokenTermination;
    struct {
        int child;
        MPI_Request request = MPI_REQUEST_NULL;
        void* buffer;
        int numElement;
        bool canBeReceive;
    } treeBranchReceive[2];
    struct {
        int child;
        MPI_Request request = MPI_REQUEST_NULL;
        void* buffer;
        int numElement;
        bool mustBeSent;
    } treeBranchSent[2];
    BranchBoundResultSolution *cacheLastBoundMessage = nullptr;
    void sendToken();
    Branch* returnBranchFromStatus(MPI_Status status);
    Branch* getBranchFromGeneralStatus(MPI_Status status);
    bool isLocalTerminate();
    void checkTermination();
public:
    long totalSendBranch = 0;
    long totalRecvBranch = 0;
    BranchBoundProblem* getBranchProblem() override;
    Branch* getRootBranch() override;
    BranchBoundResultBranch* getBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    bool isCommEnabled() override;
    MasterpoolManager(MPIDataManager &manager);
    void broadcastTerminationWithValue(bool value) override;
    double getBound() override;
    void terminate() override;
    ~MasterpoolManager();
};

#endif