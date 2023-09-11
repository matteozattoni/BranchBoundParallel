#include "workpoolmanager.h"

#include <iostream>

WorkpoolManager::WorkpoolManager(MPIDataManager &manager) : ParallelManager(), dataManager(manager)
{
    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    numberWorkpool = worldRank / WORKPOOL_WORKER;
    MPI_Comm_split(MPI_COMM_WORLD, numberWorkpool, worldRank, &workpoolComm);
    MPI_Comm_size(workpoolComm, &workpoolSize);
    MPI_Comm_rank(workpoolComm, &workpoolRank);
    receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
    MPI_Irecv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &receiveBound.request);
    if (workpoolRank == 0)
        tokenTermination.hasToken = true;
}

WorkpoolManager::~WorkpoolManager()
{
    checkTermination();
    MPI_Comm_free(&workpoolComm);
}

Branch *WorkpoolManager::getRootBranch()
{
    throw MPIUnimplementedException("Workpool getRootBranch()");
}

BranchBoundProblem *WorkpoolManager::getBranchProblem()
{
    throw MPIUnimplementedException("Workpool getBranchProblem()");
}

BranchBoundResultBranch *WorkpoolManager::getBranch()
{
    if (workpoolSize < 2)
        throw MPILocalTerminationException();

    int requestNotCompletedIndex[workpoolSize];
    MPI_Request arrayRequestNotCompleted[workpoolSize];
    int nRecvNotCompleted = 0;

    std::list<Branch*> branches;

    for (int i = 0; i < workpoolSize; i++)
    { // receive already commited and return the branch for the first receveive complteted
        if (i != workpoolRank && receiveBranch[i].request != MPI_REQUEST_NULL)
        {
            MPI_Status status;
            int isCompleted;
            MPI_Test(&receiveBranch[i].request, &isCompleted, &status);
            if (isCompleted) // here a recevice is commited AND completed
            {
                Branch *branch = dataManager.getBranchFromBuff(receiveBranch[i].branchBuffer, receiveBranch[i].numElement);
                receiveBranch[i].request = MPI_REQUEST_NULL;
                totalRecvBranch++;
                branches.push_front(branch);
            }
            else
            { // here a recevice is commited but NOT completed
                arrayRequestNotCompleted[nRecvNotCompleted] = receiveBranch[i].request;
                requestNotCompletedIndex[nRecvNotCompleted] = i;
                nRecvNotCompleted++;
            }
        }
    }

    // Here no receveive are completed, if at least one is commited we will wait for its completion
    if (nRecvNotCompleted > 0)
    { // if there is come receive commited i'll wait for it
        MPI_Status status;
        int indexArrayNotCompleted;
        MPI_Waitany(nRecvNotCompleted, arrayRequestNotCompleted, &indexArrayNotCompleted, &status);
        int index = requestNotCompletedIndex[indexArrayNotCompleted];
        Branch *branch = dataManager.getBranchFromBuff(receiveBranch[index].branchBuffer, receiveBranch[index].numElement);
        branches.push_front(branch);
        receiveBranch[index].request = MPI_REQUEST_NULL;
        totalRecvBranch++;
    }

    // Here no receveive are commited, now we ask if someone has sent a branch to us
    int isBranchIncoming;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, TAG_BRANCH, workpoolComm, &isBranchIncoming, &status);
    if (isBranchIncoming)
        branches.push_front(getBranchFromStatus(status));

    // all receive are not commited (and not send incoming) so i'll wait the first message from other (termination)
    if (branches.size() > 0) {
        return dataManager.getBranchResultFromBranches(branches);
    }

    throw MPILocalTerminationException();
}

BranchBoundResultBranch *WorkpoolManager::waitForBranch()
{
    if (workpoolSize < 2)
        throw MPIGlobalTerminationException();
    try
    {
        return getBranch();
    }
    catch (const MPILocalTerminationException &e)
    {
        MPI_Status status;
        if (tokenTermination.hasToken)
        {
            if (workpoolRank == 0 && tokenTermination.nodeColor == nodeWhite && tokenTermination.tokenColor == tokenWhite)
                throw MPIGlobalTerminationException();
            sendToken();
        }

        if (tokenTermination.hasToken && isLocalTerminate())
            sendToken();

        Branch *b = nullptr;

        while (b == nullptr)
        {
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, workpoolComm, &status);
            b = getBranchFromGeneralStatus(status);
        }
        return dataManager.getBranchResultFromBranches({b});
        
    }
}

void WorkpoolManager::sendBound(BranchBoundResultSolution *bound)
{
    if (this->bound >= bound->getSolutionResult())
        return;
    
    this->bound = (double) bound->getSolutionResult();

    if (workpoolSize < 2)
        return;

    int index = 0;
    MPI_Request sendRequest[workpoolSize - 1];
    std::pair<void *, int> pairToSend = dataManager.getBoundBuffer(bound);

    for (int i = 0; i < workpoolSize; i++)
    {
        if (i != workpoolRank)
        {
            // cout << "send bound to " << i << " val is " << bound->getSolutionResult()  << endl;
            MPI_Isend(pairToSend.first, pairToSend.second, dataManager.getBoundType(), i, TAG_BOUND, workpoolComm, &sendRequest[index]);
            index++;
        }
    }
    
    if (index > 0) {
        MPI_Status sendStatus[index];
        MPI_Waitall(index, sendRequest, sendStatus);
    }
        
}

void WorkpoolManager::loadBalance(std::function<void(BranchBoundResult *)> callback)
{
    for (int i = 0; i < workpoolSize; i++)
    {
        if (i == workpoolRank)
            continue;

        if (receiveBranch[i].request == MPI_REQUEST_NULL) // replace
        {
            MPI_Status status;
            int hasSent;
            MPI_Iprobe(i, TAG_BRANCH, workpoolComm, &hasSent, &status);
            if (hasSent)
            {
                tokenTermination.nodeColor = nodeBlack;
                int count;
                MPI_Get_count(&status, dataManager.getBranchType(), &count);
                if (count == MPI_UNDEFINED)
                    throw MPIGeneralException("WorkpoolManager::loadBalance - get count return MPI_UNDEFINED");
                void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
                MPI_Irecv(buffBranch, count, dataManager.getBranchType(), i, TAG_BRANCH, workpoolComm, &receiveBranch[i].request);
                receiveBranch[i].numElement = count;
                receiveBranch[i].branchBuffer = buffBranch;
            }
        }
    }
}

void WorkpoolManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    if (workpoolSize < 2)
        return;

    if (cacheLastBoundMessage != nullptr) {
        callback(cacheLastBoundMessage);
        cacheLastBoundMessage = nullptr;
    }

    // get bound messages if any
    receiveBoundMessage(callback);

    // load balance protocol (we don't return branch but set speculative recv)
    loadBalance(callback);
}

void WorkpoolManager::epilogue(std::function<const Branch *()> callback)
{
    if (workpoolSize < 2)
        return;
    for (int i = 0; i < workpoolSize; i++)
    {
        if (i == workpoolRank)
            continue;

        MPI_Status status;
        int testFlag;

        if (sentBranch[i].request == MPI_REQUEST_NULL)
        {
            const Branch *branch = callback();
            if (branch != nullptr)
            { // sent
                std::pair<void *, int> pair = dataManager.getBranchBuffer(branch);
                totalSendBranch++;
                MPI_Issend(pair.first, pair.second, dataManager.getBranchType(), i, TAG_BRANCH, workpoolComm, &sentBranch[i].request);
                sentBranch[i].branchBuffer = pair.first;
                sentBranch[i].numElement = pair.second;
                tokenTermination.nodeColor = nodeBlack;
            }
        }
        else
        {
            MPI_Test(&sentBranch[i].request, &testFlag, &status);
            if (testFlag)
            {
                dataManager.sentFinished(sentBranch[i].branchBuffer, sentBranch[i].numElement);
                sentBranch[i].request = MPI_REQUEST_NULL;
                const Branch *branch = callback();
                if (branch != nullptr)
                {
                    std::pair<void *, int> pair = dataManager.getBranchBuffer(branch);
                    totalSendBranch++;
                    MPI_Issend(pair.first, pair.second, dataManager.getBranchType(), i, TAG_BRANCH, workpoolComm, &sentBranch[i].request);
                    sentBranch[i].branchBuffer = pair.first;
                    sentBranch[i].numElement = pair.second;
                    //if (i < workpoolRank)
                        tokenTermination.nodeColor = nodeBlack;
                }
            }
        }
    }
}

double WorkpoolManager::getBound()
{
    return bound;
}

Branch *WorkpoolManager::getBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    if (count == MPI_UNDEFINED)
        throw MPIGeneralException("WorkpoolManager::returnBranchFromStatus - get count return MPI_UNDEFINED");
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, &status);
    Branch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    totalRecvBranch++;
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

void WorkpoolManager::receiveBoundMessage(std::function<void(BranchBoundResult *)> callback)
{
    int isBoundMessageArrived;
    MPI_Status status;
    MPI_Test(&(receiveBound.request), &isBoundMessageArrived, &status);
    if (isBoundMessageArrived)
    {
        BranchBoundResultSolution *sol = dataManager.getSolutionFromBound(receiveBound.boundBuffer);
        this->bound = std::max(this->bound, (double)sol->getSolutionResult());
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
                this->bound = std::max(this->bound, (double)sol->getSolutionResult());
                callback(sol);
                receiveBound.boundBuffer = nullptr;
            }
        }
        receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
        MPI_Irecv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &(receiveBound.request));
    }
}

bool WorkpoolManager::isCommEnabled()
{
    return workpoolComm != MPI_COMM_NULL; // always true
}

void WorkpoolManager::broadcastTerminationWithValue(bool value)
{
    if (workpoolSize < 2)
        return;
    bool val = value;
    if (workpoolRank == 0)
    {
        MPI_Bcast(&val, 1, MPI_C_BOOL, 0, workpoolComm);
    }
    else
    {
        MPI_Bcast(&val, 1, MPI_C_BOOL, 0, workpoolComm);
        if (val == true)
        {
            throw MPIGlobalTerminationException();
        }
    }
}

Branch *WorkpoolManager::getBranchFromGeneralStatus(MPI_Status status)
{
    switch (status.MPI_TAG)
    {
    case TAG_BRANCH:
    {
        return getBranchFromStatus(status);
        break;
    }
    case TAG_TOKEN:
    {
        MPI_Status branchStatus;
        int isBranchIncoming;
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_BRANCH, workpoolComm, &isBranchIncoming, &branchStatus);
        if (isBranchIncoming)
        { // there is some branch
            return getBranchFromStatus(branchStatus);
        }
        else
        { // nothing incoming we take the token
            MPI_Recv(&tokenTermination.tokenColor, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, workpoolComm, MPI_STATUS_IGNORE);
            tokenTermination.hasToken = true;
            if (workpoolRank == 0 && tokenTermination.nodeColor == nodeWhite && tokenTermination.tokenColor == tokenWhite)
            { // global termination
                if (!isLocalTerminate())
                    throw MPIGeneralException("Recv Termination before local termination!!");
                throw MPIGlobalTerminationException();
            }
            else
            {
                if(isLocalTerminate())
                    sendToken();
                return nullptr;
            }
        }
        break;
    }
    case TAG_TERMINATION:
    {
        if (!isLocalTerminate())
            throw MPIGeneralException("Recv Termination before local termination!!");
        int termination;
        MPI_Recv(&termination, 1, MPI_INT, 0, TAG_TERMINATION, workpoolComm, MPI_STATUS_IGNORE);
        throw MPIGlobalTerminationException();
        break;
    }
    case TAG_BOUND:
    {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        if (bound > result->getSolutionResult())
            delete result;
        else {
            this->bound = std::max(this->bound, (double)result->getSolutionResult());
            if (cacheLastBoundMessage != nullptr) 
                delete cacheLastBoundMessage;
            cacheLastBoundMessage = result;
            
        }
        return nullptr;
        break;
    }
    default:
    {
        throw MPIGeneralException("WorkpoolManager::waitForBranch - Unknown tag ");
        break;
    }
    }
}

void WorkpoolManager::checkTermination()
{
    int thereIsSomeMessage;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, workpoolComm, &thereIsSomeMessage, &status);
    if (thereIsSomeMessage && status.MPI_TAG == TAG_BRANCH)
        std::cout << "(" << workpoolRank <<  ")There is some branch message before deallocating WorkpoolManager" << std::endl;

    MPI_Request arrayRequest[workpoolSize];
    int nRecvReady = 0;
    for (int i = 0; i < workpoolSize; i++) // take the request of all receive commited
    {
        if (receiveBranch[i].request != MPI_REQUEST_NULL)
        {
            arrayRequest[nRecvReady++] = receiveBranch[i].request;
        }
    }

    int some;
    int index;
    if (nRecvReady > 0)
    { // if there is come receive commited i'll wait for it
        MPI_Testany(nRecvReady, arrayRequest, &index, &some, &status);
        if (some)
            std::cout << "(" << workpoolRank << ")There is some message in recvrequest in buffer before deallocating WorkpoolManager: " << status.MPI_TAG << std::endl;
    }

    int nSentReady = 0;
    for (int i = 0; i < workpoolSize; i++) // take the request of all receive commited
    {
        if (sentBranch[i].request != MPI_REQUEST_NULL)
        {
            arrayRequest[nSentReady++] = sentBranch[i].request;
        }
    }
    
    if (nSentReady > 0)
    { // if there is come receive commited i'll wait for it
        MPI_Status arrayStatus[nSentReady];
        MPI_Testall(nSentReady, arrayRequest, &some, arrayStatus);
        if (!some)
            std::cout << "(" << workpoolRank << ")There is some message in buffer before deallocating WorkpoolManager: " << status.MPI_TAG << std::endl;
    }
}

void WorkpoolManager::terminate()
{
    long totalSend;
    long totalRecv;

    if ((workpoolRank % WORKPOOL_WORKER) == 0)
    {
        for (int i = 1; i < workpoolSize; i++) // ignore the first root
        {
            int termination = 1;
            MPI_Send(&termination, 1, MPI_INT, i, TAG_TERMINATION, workpoolComm);
        }
    }

    MPI_Reduce(&totalRecvBranch, &totalRecv, 1, MPI_LONG, MPI_SUM, 0, workpoolComm);
    MPI_Reduce(&totalSendBranch, &totalSend, 1, MPI_LONG, MPI_SUM, 0, workpoolComm);

    if (workpoolRank == 0) {
        std::cout << "WORKPOOL (Node " <<  numberWorkpool << " size: " << workpoolSize << ") * Total: send: " << totalSend << " - recv: " << totalRecv <<  std::endl;
    }
}

 bool WorkpoolManager::isLocalTerminate() {
    int thereIsSomeMessage;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, TAG_BRANCH, workpoolComm, &thereIsSomeMessage, &status);
    if (thereIsSomeMessage && status.MPI_TAG == TAG_BRANCH)
        return false;

    MPI_Request arrayRequest[workpoolSize];
    int nRecvReady = 0;
    for (int i = 0; i < workpoolSize; i++) // take the request of all receive commited
    {
        if (receiveBranch[i].request != MPI_REQUEST_NULL)
        {
            arrayRequest[nRecvReady++] = receiveBranch[i].request;
        }
    }

    int some;
    int index;
    if (nRecvReady > 0)
    { // if there is come receive commited i'll wait for it
        MPI_Testany(nRecvReady, arrayRequest, &index, &some, &status);
        if (some)
            return false;
    }

/*     nRecvReady = 0;
    for (int i = 0; i < workpoolSize; i++) // take the request of all receive commited
    {
        if (sentBranch[i].request != MPI_REQUEST_NULL)
        {
            arrayRequest[nRecvReady++] = sentBranch[i].request;
        }
    }

    if (nRecvReady > 0)
    { // if there is come receive commited i'll wait for it
        MPI_Testany(nRecvReady, arrayRequest, &index, &some, &status);
        if (some)
            return false;
    } */

    return true;
 }

 int WorkpoolManager::getIdentity() {
    return workpoolRank;
 }