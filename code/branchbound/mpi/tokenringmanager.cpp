#include "tokenringmanager.h"
#include <iostream>

TokenRingManager::TokenRingManager(MPIDataManager &manager) : MPIManager(manager)
{
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    nextRankToSend = (rank + 1) % size;
    tokenTermination.hasToken = rank == 0;
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
    while (!listOfMessage.empty())
    {
        MPIMessage *message = listOfMessage.back();
        if (message->tag == BOUND)
        {
            BranchBoundResultSolution *result = dataManager.getSolutionFromBound(message->buffer);
            callback(result);
            listOfMessage.pop_back();
            delete message;
        }
        else
        {
            throw MPIUnimplementedException("MasterpoolManager::prologue: listOfMessage tag unhandled");
        }
    }
    // BRANCH RECEIVED
    if (branchReceived.request == MPI_REQUEST_NULL)
    {
        int isBranchIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, MPI_COMM_WORLD, &isBranchIncoming, &status);
        if (isBranchIncoming)
        {
            int countElem;
            MPI_Get_count(&status, dataManager.getBranchType(), &countElem);
            if (countElem == MPI_UNDEFINED)
                throw MPIGeneralException("MasterpoolManager::prologue - MPI_Get_count return MPI_UNDEFINED");
            branchReceived.buffer = dataManager.getEmptyBranchElementBuff(countElem);
            branchReceived.numElement = countElem;
            MPI_Irecv(branchReceived.buffer, countElem, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &branchReceived.request);
        }
    }

    int isBoundIncoming;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, BOUND, MPI_COMM_WORLD, &isBoundIncoming, &status);
    while (isBoundIncoming)
    {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        callback(result);
        MPI_Iprobe(MPI_ANY_SOURCE, BOUND, MPI_COMM_WORLD, &isBoundIncoming, &status);
    }
}

void TokenRingManager::epilogue(std::function<const Branch *()> callback)
{
    if (branchSent.request == MPI_REQUEST_NULL)
    {
        const Branch *branch = callback();
        if (branch != nullptr)
        {
            std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branch);
            branchSent.buffer = pairToSend.first;
            branchSent.numElement = pairToSend.second;
            totalSendBranch++;
            MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, MPI_COMM_WORLD, &branchSent.request);
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
                MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, MPI_COMM_WORLD, &branchSent.request);
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
            BranchBoundResultBranch *result = dataManager.getBranchFromBuff(branchReceived.buffer, branchReceived.numElement);
            totalRecvBranch++;
            branchReceived.request = MPI_REQUEST_NULL;
            return result;
        }
    }

    if (branchReceived.request == MPI_REQUEST_NULL && size > 1)
    {
        int isRecvIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, MPI_COMM_WORLD, &isRecvIncoming, &status);
        if (isRecvIncoming)
            return returnBranchFromStatus(status);
    }

    throw MPILocalTerminationException();
}

BranchBoundResultBranch *TokenRingManager::waitForBranch()
{
    if (size < 2)
        throw MPIGlobalTerminationException();
    if (tokenTermination.hasToken)
    {
        int isBranchIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, MPI_COMM_WORLD, &isBranchIncoming, &status);
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

    if (tokenTermination.hasToken && isLocalTerminate())
        sendToken();

    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
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
    MPI_Send(pairToSend.first, pairToSend.second, dataManager.getBoundType(), nextRankToSend, BOUND, MPI_COMM_WORLD);
}

BranchBoundResultBranch *TokenRingManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    if (count == MPI_UNDEFINED)
        throw MPIGeneralException("MasterpoolManager::returnBranchFromStatus get_count return MPI_UNDEFINED");
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
    BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    totalRecvBranch++;
    return branch;
}

BranchBoundResultBranch *TokenRingManager::getResultFromStatus(MPI_Status status)
{
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
            BranchBoundResultBranch *result = dataManager.getBranchFromBuff(branchReceived.buffer, branchReceived.numElement);
            totalRecvBranch++;
            branchReceived.request = MPI_REQUEST_NULL;
            return result;
        }

        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, MPI_COMM_WORLD, &isBranchIncoming, &branchStatus);
        if (isBranchIncoming)
        { // there is some branch
            return returnBranchFromStatus(branchStatus);
        }
        else
        { // nothing incoming we take the token
            MPI_Recv(&tokenTermination.tokenColor, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            tokenTermination.hasToken = true;
            if (rank == 0 && tokenTermination.tokenColor == tokenWhite)
            {                                            // global termination
                if (!isLocalTerminate())
                    throw MPIGeneralException("Recv Termination before local termination!!");
                for (int i = 1; i < size; i++) // ignore the first root
                {
                    int termination = 1;
                    MPI_Send(&termination, 1, MPI_INT, i, TERMINATION, MPI_COMM_WORLD);
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
        MPI_Recv(&termination, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        throw MPIGlobalTerminationException();
        break;
    }
    case BOUND:
    {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        // std::cout << "final bound: " << result->getSolutionResult() << std::endl;
        MPIMessage *message = new MPIMessage(buffer, 1, BOUND);
        this->bound = std::max(this->bound, (double)result->getSolutionResult());
        listOfMessage.push_back(message);
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
        return;


    tokenTermination.tokenColor = rank == 0 && tokenTermination.nodeColor == nodeWhite ? tokenWhite : tokenTermination.tokenColor;

    if (tokenTermination.nodeColor == nodeWhite)
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, nextRankToSend, TOKEN, MPI_COMM_WORLD);
    else
    {
        tokenTermination.tokenColor = tokenBlack;
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, nextRankToSend, TOKEN, MPI_COMM_WORLD);
    }
    tokenTermination.nodeColor = nodeWhite;
    tokenTermination.hasToken = false;
}


void TokenRingManager::checkTermination()
{
    int someMessage;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &someMessage, &status);
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
