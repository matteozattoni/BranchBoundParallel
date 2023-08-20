#include "workpoolmanager.h"

#include <iostream>

WorkpoolManager::WorkpoolManager(MPIDataManager &manager): MPIManager(manager)
{
    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    const int myworkpool = worldRank / WORKPOOL_WORKER;
    MPI_Comm_split(MPI_COMM_WORLD, myworkpool, worldRank, &workpoolComm);
    MPI_Comm_size(workpoolComm, &workpoolSize);
    MPI_Comm_rank(workpoolComm, &workpoolRank);
    receiveBranch[workpoolRank].ready = false;
    sentBranch[workpoolRank].isSent = true;
    receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
    MPI_Irecv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &receiveBound.request);
    if (workpoolRank == 0)
        tokenTermination.hasToken = true;
}

WorkpoolManager::~WorkpoolManager()
{
}

const Branch *WorkpoolManager::getRootBranch()
{
    throw MPIUnimplementedException("Workpool getRootBranch()");
}

BranchBoundProblem *WorkpoolManager::getBranchProblem() {
    throw MPIUnimplementedException("Workpool getBranchProblem()");
}

BranchBoundResultBranch *WorkpoolManager::getBranch()
{
    if (workpoolSize < 2)
        throw MPILocalTerminationException();
    
    for (int i = 0; i < workpoolSize; i++)
    { // receive already commited (see if completed)
        if (i != workpoolRank && receiveBranch[i].ready == true)
        {
            MPI_Status status;
            int isCompleted;
            MPI_Test(&receiveBranch[i].request, &isCompleted, &status);
            if (isCompleted)
            { // here a recevice is commited AND completed
                BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(receiveBranch[i].branchBuffer, receiveBranch[i].count);
                receiveBranch[i].ready = false;
                return branch;
            }
        }
    }
    MPI_Request arrayRequest[workpoolSize];
    int nRecvReady = 0;
    for (int i = 0; i < workpoolSize; i++) // take the request of all receive commited
    {
        if (receiveBranch[i].ready)
        {
            arrayRequest[nRecvReady++] = receiveBranch[i].request;
        }
    }

    MPI_Status status;
    int index;
    if (nRecvReady > 0)
    { // if there is come receive commited i'll wait for it
        MPI_Waitany(nRecvReady, arrayRequest, &index, &status);
        int count;
        BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(receiveBranch[index].branchBuffer, receiveBranch[index].count);
        receiveBranch[index].ready = false;
        return branch;
    }
    else
    { // all receive are not commited (and not send incoming) so i'll wait the first message from other (termination)
        throw MPILocalTerminationException();
    }
}

BranchBoundResultBranch* WorkpoolManager::waitForBranch() {
    if (workpoolSize < 2)
        throw MPIGlobalTerminationException();

    MPI_Status status;
    int isBranchIncoming;
    if (tokenTermination.hasToken)
    {
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_BRANCH, workpoolComm, &isBranchIncoming, &status); // there are send to us
        if (isBranchIncoming)
        {
            return returnBranchFromStatus(status);
        }
        else
        {
            if (workpoolRank == 0 && tokenTermination.nodeColor == nodeWhite && tokenTermination.tokenColor == tokenWhite)
                throw MPIGlobalTerminationException();
            sendToken();
            return waitForBranch();
        }
    }
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, workpoolComm, &status);
    switch (status.MPI_TAG)
    {
    case TAG_BRANCH: {
        return returnBranchFromStatus(status);
        break;
    }
    case TAG_TOKEN: {
        MPI_Status branchStatus;
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_BRANCH, workpoolComm, &isBranchIncoming, &branchStatus);
        if (isBranchIncoming)
        { // there is some branch
            return returnBranchFromStatus(branchStatus);
        }
        else
        { // nothing incoming we take the token
            MPI_Recv(&tokenTermination.tokenColor, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, workpoolComm, MPI_STATUS_IGNORE);
            tokenTermination.hasToken = true;
            if (workpoolRank == 0 && tokenTermination.tokenColor == tokenWhite)
            {                                          // global termination
                for (int i = 1; i < workpoolSize; i++) // ignore the first root
                {
                    int termination;
                    MPI_Send(&termination, 1, MPI_INT, i, TAG_TERMINATION, workpoolComm);
                }
                throw MPIGlobalTerminationException();
            }
            else
            {
                sendToken();
                return waitForBranch();
            }
        }
        break;
    }
    case TAG_TERMINATION: {
        int termination;
        MPI_Recv(&termination, 1, MPI_INT, 0, TAG_TERMINATION, workpoolComm, MPI_STATUS_IGNORE);
        throw MPIGlobalTerminationException();
        break;
    }
    case TAG_BOUND: {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        this->bound = std::max(this->bound, (double) result->getSolutionResult());
        MPIMessage* message = new MPIMessage(buffer, 1, TAG_BOUND);
        listOfMessage.push_back(message);
        break;
    }
    default: {
        throw MPIGeneralException("WorkpoolManager::waitForBranch - Unknown tag: " + status.MPI_TAG);
        break;
    }
    }
    throw MPIGeneralException("WorkpoolManager::waitForBranch - the last code line should'nt be reach");
}

void WorkpoolManager::sendBound(BranchBoundResultSolution *bound)
{
    if (workpoolSize < 2)
        return;
    this->bound = std::max(this->bound, (double) bound->getSolutionResult());
    int index = 0;
    MPI_Request sendRequest[workpoolSize - 1];
    MPI_Status sendStatus[workpoolSize - 1];
    std::pair<void *, int> pairToSend = dataManager.getBoundBuffer(bound);

    for (int i = 0; i < workpoolSize; i++)
    {
        if (i != workpoolRank)
        {
            //cout << "send bound to " << i << " val is " << bound->getSolutionResult()  << endl;
            MPI_Isend(pairToSend.first, pairToSend.second, dataManager.getBoundType(), i, TAG_BOUND, workpoolComm, &sendRequest[index++]);
        }
    }
    MPI_Waitall(workpoolSize - 1, sendRequest, sendStatus);
}

void WorkpoolManager::loadBalance(std::function<void(BranchBoundResult *)> callback)
{
    for (int i = 0; i < workpoolSize; i++)
    {
        MPI_Status status;
        int hasSent;
        if (receiveBranch[i].ready == false && i != workpoolRank)
        {
            MPI_Iprobe(i, TAG_BRANCH, workpoolComm, &hasSent, &status);
            if (hasSent)
            {
                int count;
                MPI_Get_count(&status, dataManager.getBranchType(), &count);
                void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
                MPI_Irecv(buffBranch, count, dataManager.getBranchType(), i, TAG_BRANCH, workpoolComm, &receiveBranch[i].request);
                receiveBranch[i].ready = true;
                receiveBranch[i].count = count;
                receiveBranch[i].branchBuffer = buffBranch;
            }
        }
    }
}

void WorkpoolManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    if (workpoolSize < 2)
        return;

    while (!listOfMessage.empty()) {
        MPIMessage *message = listOfMessage.back();
        if (message->tag == TAG_BOUND) {
            BranchBoundResultSolution *result = dataManager.getSolutionFromBound(message->buffer);
            callback(result);
            listOfMessage.pop_back();
            delete message;
        }else {
            throw MPIUnimplementedException("MasterpoolManager::prologue: listOfMessage tag unhandled");
        }
    }
    int boundFlag;
    MPI_Status status;

    // get bound messages if any
    receiveBoundMessage(callback);

    // load balance protocol
    loadBalance(callback);
}

void WorkpoolManager::epilogue(std::function<const Branch *()> callback)
{
    if (workpoolSize < 2)
        return;
    for (int i = 0; i < workpoolSize; i++)
    {
        MPI_Status status;
        int hasSent, testFlag;
        if (i != workpoolRank && sentBranch[i].isSent == false)
        {
            const Branch *branch = callback();
            if (branch != nullptr)
            { // sent
                std::pair<void *, int> pair = dataManager.getBranchBuffer(branch);
                MPI_Issend(pair.first, pair.second, dataManager.getBranchType(), i, TAG_BRANCH, workpoolComm, &sentBranch[i].request);
                sentBranch[i].isSent = true;
                sentBranch[i].sentBufferAndCount = pair;
                if (i < workpoolRank)
                    tokenTermination.nodeColor = nodeBlack;
            }
            else
            {
                break;
            }
        }
        else if (i != workpoolRank)
        {
            MPI_Test(&sentBranch[i].request, &testFlag, &status);
            if (testFlag)
            {
                if (i < workpoolRank)
                    tokenTermination.nodeColor = nodeBlack;
                dataManager.sentFinished(sentBranch[i].sentBufferAndCount.first, sentBranch[i].sentBufferAndCount.second);
                sentBranch[i].isSent = false;
                const Branch *branch = callback();
                if (branch != nullptr)
                {
                    std::pair<void *, int> pair = dataManager.getBranchBuffer(branch);
                    MPI_Issend(pair.first, pair.second, dataManager.getBranchType(), i, TAG_BRANCH, workpoolComm, &sentBranch[i].request);
                    sentBranch[i].isSent = true;
                    sentBranch[i].sentBufferAndCount = pair;
                }
                else
                {
                    break;
                }
            }
        }
    }
}

double WorkpoolManager::getBound() {
    return bound;
}

BranchBoundResultBranch *WorkpoolManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, &status);
    BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    return branch;
}

void WorkpoolManager::sendToken()
{
    if (!tokenTermination.hasToken)
        return;
    tokenTermination.tokenColor = workpoolRank == 0 ? tokenWhite : tokenTermination.tokenColor;
    int rankToSend = (workpoolRank + 1) % workpoolSize;
    // cout << "send token to " << rankToSend << endl;
    if (tokenTermination.nodeColor == nodeWhite)
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, rankToSend, TAG_TOKEN, workpoolComm);
    else
    {
        tokenTermination.tokenColor = tokenBlack;
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, rankToSend, TAG_TOKEN, workpoolComm);
    }
    tokenTermination.nodeColor = nodeWhite;
    tokenTermination.hasToken = false;
}

void WorkpoolManager::receiveBoundMessage(std::function<void(BranchBoundResult *)> callback) {
    int isBoundMessageArrived;
    MPI_Status status;
    MPI_Test(&(receiveBound.request), &isBoundMessageArrived, &status);
    if (isBoundMessageArrived)
    {
        BranchBoundResultSolution *sol = dataManager.getSolutionFromBound(receiveBound.boundBuffer);
        this->bound = std::max(this->bound, (double) sol->getSolutionResult());
        callback(sol);
        receiveBound.boundBuffer = nullptr;
        // there are more?
        while (isBoundMessageArrived)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &isBoundMessageArrived, &status);
            if (isBoundMessageArrived)
            {
                // get buffer for bound
                receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
                MPI_Recv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, &status);
                BranchBoundResultSolution *sol = dataManager.getSolutionFromBound(receiveBound.boundBuffer);
                this->bound = std::max(this->bound, (double) sol->getSolutionResult());
                callback(sol);
                receiveBound.boundBuffer = nullptr;
            }
        }
        receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
        MPI_Irecv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &(receiveBound.request));
    }
}

bool WorkpoolManager::isCommEnabled() {
    return workpoolComm != MPI_COMM_NULL; // always true
}

void WorkpoolManager::broadcastTerminationWithValue(bool value) {
    if (workpoolSize < 2)
        return;
    bool val = value;
    if (workpoolRank == 0) {
        MPI_Bcast(&val, 1, MPI_C_BOOL, 0, workpoolComm);
    } else {
        MPI_Bcast(&val, 1, MPI_C_BOOL, 0, workpoolComm);
        if (val == true) {
            throw MPIGlobalTerminationException();
        }
    }
}