#include "tokenringmanager.h"
#include <iostream>

TokenRingManager::TokenRingManager(MPIDataManager &manager) : MPIManager(manager)
{
    int worldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    numberWorkpool = worldRank / WORKPOOL_WORKER;
    MPI_Comm_split(MPI_COMM_WORLD, numberWorkpool, worldRank, &comm);
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    tokenTermination.hasToken = rank == 0;
    nextRankToSend = (rank + 1) % size;
    std::cout << "world rank: " << worldRank << " tokenpool " << numberWorkpool << " rank in this pool " << rank << " next rank to send: " << nextRankToSend << std::endl;
}

TokenRingManager::~TokenRingManager()
{
    checkTermination();
}

BranchBoundProblem *TokenRingManager::getBranchProblem()
{
    throw MPIUnimplementedException("MPIGlobalManager::getBranchProblem()");
}

Branch *TokenRingManager::getRootBranch()
{
    throw MPIUnimplementedException("MPIGlobalManager::getRootBranch()");
}

void TokenRingManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    if (rank == 1 && numberWorkpool == 1)
        std::cout << "rank 3 prologue" << std::endl;
    if (size < 2)
        return;

    if (cacheLastBoundMessage != nullptr) {
        callback(cacheLastBoundMessage);
        cacheLastBoundMessage = nullptr;
    }

    // BRANCH RECEIVED
    if (branchReceived.request == MPI_REQUEST_NULL)
    {
        int isBranchIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, comm, &isBranchIncoming, &status);
        if (isBranchIncoming)
        {
            if (rank == 1 && numberWorkpool == 1)
                std::cout << "got branch" << std::endl;
            int countElem;
            MPI_Get_count(&status, dataManager.getBranchType(), &countElem);
            if (countElem == MPI_UNDEFINED)
                throw MPIGeneralException("MasterpoolManager::prologue - MPI_Get_count return MPI_UNDEFINED");
            branchReceived.buffer = dataManager.getEmptyBranchElementBuff(countElem);
            branchReceived.numElement = countElem;
            MPI_Irecv(branchReceived.buffer, countElem, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, comm, &branchReceived.request);
        }
    }

    int isBoundIncoming;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, BOUND, comm, &isBoundIncoming, &status);
    while (isBoundIncoming)
    {
        if (rank == 1 && numberWorkpool == 1)
                std::cout << "got bound" << std::endl;
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, comm, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        callback(result);
        MPI_Iprobe(MPI_ANY_SOURCE, BOUND, comm, &isBoundIncoming, &status);
    }
}

void TokenRingManager::epilogue(std::function<const Branch *()> callback)
{
    if (rank == 1 && numberWorkpool == 1)
        std::cout << "rank 3 epilogue" << std::endl;
    if (size < 2)
        return;
    
    if (branchSent.request == MPI_REQUEST_NULL)
    {
        const Branch *branch = callback();
        if (branch != nullptr)
        {
            std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branch);
            branchSent.buffer = pairToSend.first;
            branchSent.numElement = pairToSend.second;
            totalSendBranch++;
            if (rank == 0 && numberWorkpool == 1)
                std::cout << rank << " branch sent" << std::endl;
            MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, comm, &branchSent.request);
            tokenTermination.nodeColor = nodeBlack;
        }
    }
    else
    {
        int isSentFinished;
        MPI_Test(&branchSent.request, &isSentFinished, MPI_STATUS_IGNORE);
        if (isSentFinished)
        {
            dataManager.sentFinished(branchSent.buffer, branchSent.numElement);
            branchSent.request = MPI_REQUEST_NULL;
            const Branch *branch = callback();
            if (branch != nullptr)
            {
                std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branch);
                branchSent.buffer = pairToSend.first;
                branchSent.numElement = pairToSend.second;
                totalSendBranch++;
                if (rank == 0 && numberWorkpool == 1)
                    std::cout << rank << " branch sent" << std::endl;
                MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, comm, &branchSent.request);
                tokenTermination.nodeColor = nodeBlack;
            }
        }
    }
}

BranchBoundResultBranch *TokenRingManager::getBranch()
{
    if (branchReceived.request != MPI_REQUEST_NULL && size > 1)
    {
        int isRecvFinished;
        MPI_Status status;
        MPI_Test(&branchReceived.request, &isRecvFinished, &status);
        
        if (isRecvFinished)
        {
            std::list<Branch*> branches;
            Branch* branch = dataManager.getBranchFromBuff(branchReceived.buffer, branchReceived.numElement);
            branches.push_front(branch);
            BranchBoundResultBranch *result = dataManager.getBranchResultFromBranches(branches);
            totalRecvBranch++;
            branchReceived.request = MPI_REQUEST_NULL;
            return result;
        }

    }

    throw MPILocalTerminationException();
}

BranchBoundResultBranch *TokenRingManager::waitForBranch()
{
    if (rank == 1 && numberWorkpool == 1)
        std::cout << "rank 3 wait for branch" << std::endl;
    if (size < 2)
        throw MPIGlobalTerminationException();
    
    if (tokenTermination.hasToken)
    {
        int isBranchIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, comm, &isBranchIncoming, &status);
        if (isBranchIncoming)
        {
            return returnBranchFromStatus(status);
        }
        else
        {
            if (rank == 0 && tokenTermination.nodeColor == nodeWhite && tokenTermination.tokenColor == tokenWhite)
                throw MPIGlobalTerminationException();
            sendToken();
            return waitForBranch();
        }
    }

    MPI_Status status;
    std::cout << rank << " before probe has token ring: " << (tokenTermination.hasToken ? "true" : "false") << std::endl;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
    return getResultFromStatus(status);
}

void TokenRingManager::sendBound(BranchBoundResultSolution *bound)
{

    if (this->bound >= bound->getSolutionResult())
        return;

    this->bound = bound->getSolutionResult();
    
    if (size < 2)
        return;

    std::pair<void *, int> pairToSend = dataManager.getBoundBuffer(bound);
    MPI_Send(pairToSend.first, pairToSend.second, dataManager.getBoundType(), nextRankToSend, BOUND, comm);
}

BranchBoundResultBranch *TokenRingManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    if (count == MPI_UNDEFINED)
        throw MPIGeneralException("MasterpoolManager::returnBranchFromStatus get_count return MPI_UNDEFINED");
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, comm, &status);
    Branch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    totalRecvBranch++;
    return dataManager.getBranchResultFromBranches({branch});
}

BranchBoundResultBranch *TokenRingManager::getResultFromStatus(MPI_Status status)
{
    if (rank == 1 && numberWorkpool == 1)
        std::cout << "rank 3 got messsage " << status.MPI_TAG << std::endl;
    switch (status.MPI_TAG)
    {
    case BRANCH:
    {
        return returnBranchFromStatus(status);
        break;
    }
    case TOKEN:
    {
        int isBranchIncoming;
        int isBranchReceived;
        MPI_Status branchStatus;
        if (branchReceived.request != MPI_REQUEST_NULL)
        {
            MPI_Test(&branchReceived.request, &isBranchReceived, &branchStatus);
            BranchBoundResultBranch *result = dataManager.getBranchResultFromBranches({dataManager.getBranchFromBuff(branchReceived.buffer, branchReceived.numElement)});
            totalRecvBranch++;
            branchReceived.request = MPI_REQUEST_NULL;
            return result;
        }

        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, comm, &isBranchIncoming, &branchStatus);
        if (isBranchIncoming)
        { // there is some branch
            return returnBranchFromStatus(branchStatus);
        }
        else
        { // nothing incoming we take the token
            MPI_Recv(&tokenTermination.tokenColor, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, comm, MPI_STATUS_IGNORE);
            tokenTermination.hasToken = true;
            std::cout << rank << " tokenring token " << std::endl;
            if (rank == 0 && tokenTermination.tokenColor == tokenWhite)
            {                                            // global termination
                if (!isLocalTerminate())
                    throw MPIGeneralException("Recv Termination before local termination!!");
                for (int i = 1; i < size; i++) // ignore the first root
                {
                    int termination = 1;
                    MPI_Send(&termination, 1, MPI_INT, i, TERMINATION, comm);
                }
                throw MPIGlobalTerminationException();
            }
            else
            {
                if (isLocalTerminate())
                    sendToken();
                return waitForBranch();
            }
        }
        break;
    }
    case TERMINATION:
    {
        int termination;
        MPI_Recv(&termination, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, comm, MPI_STATUS_IGNORE);
        throw MPIGlobalTerminationException();
        break;
    }
    case BOUND:
    {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, comm, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        if (bound > result->getSolutionResult())
            delete result;
        else {
            this->bound = std::max(this->bound, (double)result->getSolutionResult());
            if (cacheLastBoundMessage != nullptr) 
                delete cacheLastBoundMessage;
            cacheLastBoundMessage = result;
            
        }
        return waitForBranch();
        break;
    }
    default:
    {
        throw MPIGeneralException("MasterpoolManager::waitForBranch - Unknown tag ");
        break;
    }
    }
}

bool TokenRingManager::isLocalTerminate() {
    if (branchReceived.request != MPI_REQUEST_NULL)
        return false;
    return true;
}

void TokenRingManager::sendToken()
{
    if (!tokenTermination.hasToken)
        throw 1;


    tokenTermination.tokenColor = rank == 0 && tokenTermination.nodeColor == nodeWhite ? tokenWhite : tokenTermination.tokenColor;

    if (tokenTermination.nodeColor == nodeWhite)
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, nextRankToSend, TOKEN, comm);
    else
    {
        tokenTermination.tokenColor = tokenBlack;
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, nextRankToSend, TOKEN, comm);
    }
    tokenTermination.nodeColor = nodeWhite;
    tokenTermination.hasToken = false;
}


void TokenRingManager::checkTermination()
{
    int someMessage;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &someMessage, &status);
    if (someMessage && status.MPI_TAG == BRANCH)
        std::cout << "There is some branch message before deallocating Mastermanager" << std::endl;
    if (branchReceived.request != MPI_REQUEST_NULL)
        std::cout << "branchReceived request is not null! Mastermanager " << std::endl;
    if (branchSent.request != MPI_REQUEST_NULL)
    {
        int isFinished;
        MPI_Test(&branchSent.request, &isFinished, MPI_STATUS_IGNORE);
        if (!isFinished)
            std::cout << "branchSent request has not been read! Mastermanager " << std::endl;
    }
}
