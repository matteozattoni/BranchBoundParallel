#ifndef MPIGLOBALMANAGER_H
#define MPIGLOBALMANAGER_H

#include "mpimanager.h"
#include "workpoolmanager.h"
#include "mpiexceptions.h"
#include "mpimessage.h"
#include <vector>

#define GLOBAL_MASTER_RANK 0

class MasterpoolManager: public MPIManager
{
private:
    double bound = 0.0;
    int masterpoolSize;
    int masterpoolRank;
    int nextRankToSend;
    MPIManager *workpoolManager;
    MPI_Comm masterpoolComm;
    enum tagMessage { BRANCH, BRANCH_REQUEST, BOUND, TOKEN, TERMINATION};
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
    } branchReceived;
    struct {
        MPI_Request request;
    } boundSent;
    struct {
        eTokenColor tokenColor = tokenWhite;
        eNodeColor nodeColor = nodeBlack;
        bool hasToken = false;
        MPI_Request request;
    } tokenTermination;
    std::vector<MPIMessage*> listOfMessage;
    void sendToken();
    BranchBoundResultBranch * returnBranchFromStatus(MPI_Status status);
    BranchBoundResultBranch* getResultFromStatus(MPI_Status status);
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