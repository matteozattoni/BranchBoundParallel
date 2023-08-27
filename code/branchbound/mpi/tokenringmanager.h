#ifndef TOKENRINGMANAGER_H
#define TOKENRINGMANAGER_H

#include <vector>
#include "mpimanager.h"
#include "mpimessage.h"

class TokenRingManager: public MPIManager
{
private:
    double bound = 0.0;
    int rank;
    int size;
    int nextRankToSend;
    long totalSendBranch = 0;
    long totalRecvBranch = 0;
    enum tagMessage { BRANCH, BRANCH_REQUEST, BOUND, TOKEN, TERMINATION};
    enum eTokenColor {tokenWhite, tokenBlack};
    enum eNodeColor {nodeWhite , nodeBlack};
    struct {
        MPI_Request request = MPI_REQUEST_NULL;
        void* buffer;
        int numElement;
    } branchReceived;
    struct {
        MPI_Request request = MPI_REQUEST_NULL;
        void* buffer;
        int numElement;
    } branchSent;
    struct {
        eTokenColor tokenColor = tokenWhite;
        eNodeColor nodeColor = nodeBlack;
        bool hasToken = false;
        MPI_Request request;
    } tokenTermination;
    /* data */
    std::vector<MPIMessage*> listOfMessage;
    BranchBoundResultBranch * returnBranchFromStatus(MPI_Status status);
    BranchBoundResultBranch* getResultFromStatus(MPI_Status status);
    bool isLocalTerminate();
    void sendToken();
public:
    BranchBoundProblem* getBranchProblem() override;
    Branch* getRootBranch() override;
    BranchBoundResultBranch* getBranch() override;
    BranchBoundResultBranch* waitForBranch() override;
    void prologue(std::function<void(BranchBoundResult*)>) override;
    void epilogue(std::function<const Branch*()>) override;
    void sendBound(BranchBoundResultSolution* bound) override;
    bool isCommEnabled() override {return true;} ;
    void broadcastTerminationWithValue(bool value) override {};
    double getBound() override {return bound;};
    void terminate() override {};
    TokenRingManager(MPIDataManager &manager);
    ~TokenRingManager();
};


#endif