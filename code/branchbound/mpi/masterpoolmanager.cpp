#include "masterpoolmanager.h"

#include <cmath>
#include <iostream>

MasterpoolManager::MasterpoolManager(MPIDataManager &manager) : MPIManager(manager)
{
    int worldSize;
    int worldRank;
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    double doublepoolnumbers = (double)worldSize / (double)WORKPOOL_WORKER;
    int poolnumber = ceil(doublepoolnumbers);
    int rankspool[poolnumber];
    int index = 0;
    for (int i = 0; i < worldSize; i++)
    {
        if ((i % WORKPOOL_WORKER) == 0)
            rankspool[index++] = i;
    }
    MPI_Group worldGroup;
    MPI_Group masterGroup;
    MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
    MPI_Group_incl(worldGroup, poolnumber, rankspool, &masterGroup);
    MPI_Comm_create_group(MPI_COMM_WORLD, masterGroup, 0, &masterpoolComm);
    if (masterpoolComm != MPI_COMM_NULL) {
        MPI_Comm_size(masterpoolComm, &masterpoolSize);
        MPI_Comm_rank(masterpoolComm, &masterpoolRank);
        nextRankToSend = (masterpoolRank + 1) % masterpoolSize;
        tokenTermination.hasToken = masterpoolRank == 0 ? true : false;
    }
    MPI_Group_free(&masterGroup);
    /*     std::cout << "World rank: " << worldRank;
        if (masterpoolComm == MPI_COMM_NULL) {
            std::cout << " - I'm NOT in master pool" << std::endl;
        } else
            std::cout << " - I'm IN master pool!" << std::endl; */
}

MasterpoolManager::~MasterpoolManager()
{
    if (masterpoolComm != MPI_COMM_NULL)
        MPI_Comm_free(&masterpoolComm);
}

BranchBoundProblem *MasterpoolManager::getBranchProblem()
{
    throw MPIUnimplementedException("MPIGlobalManager::getBranchProblem()");
}

const Branch *MasterpoolManager::getRootBranch()
{
    throw MPIUnimplementedException("MPIGlobalManager::getRootBranch()");
}

BranchBoundResultBranch *MasterpoolManager::getBranch()
{
    if (branchReceived.request != MPI_REQUEST_NULL && masterpoolSize > 1)
    {
        int isRecvFinished;
        MPI_Status status;
        MPI_Test(&branchReceived.request, &isRecvFinished, &status);
        if (isRecvFinished)
        {
            BranchBoundResultBranch *result = dataManager.getBranchFromBuff(branchReceived.buffer, branchReceived.numElement);
            branchReceived.request = MPI_REQUEST_NULL;
            return result;
        }
    }
    throw MPILocalTerminationException();
}

BranchBoundResultBranch *MasterpoolManager::waitForBranch()
{
    try
    {
        return getBranch();
    }
    catch (const MPILocalTerminationException &e)
    {
        if (masterpoolSize < 2)
            throw MPIGlobalTerminationException();

        if (tokenTermination.hasToken)
        {
            int isBranchIncoming;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, masterpoolComm, &isBranchIncoming, &status);
            if (isBranchIncoming)
            {
                return returnBranchFromStatus(status);
            }
            else
            {
                if (masterpoolRank == 0 && tokenTermination.nodeColor == nodeWhite && tokenTermination.tokenColor == tokenWhite)
                    throw MPIGlobalTerminationException();
                sendToken();
                return waitForBranch();
            }
        }
        MPI_Status status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, masterpoolComm, &status);
        switch (status.MPI_TAG)
        {
        case BRANCH: {
            return returnBranchFromStatus(status);
            break;
        }
        case TOKEN: {
            int isBranchIncoming;
            MPI_Status branchStatus;
            MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, masterpoolComm, &isBranchIncoming, &branchStatus);
            if (isBranchIncoming)
            { // there is some branch
                return returnBranchFromStatus(branchStatus);
            }
            else
            { // nothing incoming we take the token
                MPI_Recv(&tokenTermination.tokenColor, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, MPI_STATUS_IGNORE);
                tokenTermination.hasToken = true;
                if (masterpoolRank == 0 && tokenTermination.tokenColor == tokenWhite)
                {                                          // global termination
                    for (int i = 1; i < masterpoolSize; i++) // ignore the first root
                    {
                        int termination;
                        MPI_Send(&termination, 1, MPI_INT, i, TERMINATION, masterpoolComm);
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
        case TERMINATION: {
            int termination;
            MPI_Recv(&termination, 1, MPI_INT, 0, TERMINATION, masterpoolComm, MPI_STATUS_IGNORE);
            throw MPIGlobalTerminationException();
            break;
        }
        case BOUND: {
            void *buffer = dataManager.getEmptybBoundBuff();
            MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, MPI_STATUS_IGNORE);
            BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
            //std::cout << "final bound: " << result->getSolutionResult() << std::endl;
            MPIMessage* message = new MPIMessage(buffer, 1, BOUND);
            this->bound = std::max(this->bound, (double) result->getSolutionResult());
            listOfMessage.push_back(message);
            return waitForBranch();
            break;
        }
        default: {
            throw MPIGeneralException("MasterpoolManager::waitForBranch - Unknown tag: " + status.MPI_TAG);
            break;
        }
        }
    } catch (const std::exception &e) {
         throw e;
    }
    throw MPIGeneralException("MasterpoolManager::waitForBranch - the last code line should'nt be reach");
}

void MasterpoolManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
        return;

    while (!listOfMessage.empty()) {
        MPIMessage *message = listOfMessage.back();
        if (message->tag == BOUND) {
            BranchBoundResultSolution *result = dataManager.getSolutionFromBound(message->buffer);
            callback(result);
            listOfMessage.pop_back();
            delete message;
        }else {
            throw MPIUnimplementedException("MasterpoolManager::prologue: listOfMessage tag unhandled");
        }
    }

    // BRANCH RECEIVED
    if (branchReceived.request == MPI_REQUEST_NULL)
    {
        int isBranchIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, masterpoolComm, &isBranchIncoming, &status);
        if (isBranchIncoming)
        {
            int countElem;
            MPI_Get_count(&status, dataManager.getBranchType(), &countElem);
            branchReceived.buffer = dataManager.getEmptyBranchElementBuff(countElem);
            branchReceived.numElement = countElem;
            MPI_Irecv(branchReceived.buffer, countElem, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, &branchReceived.request);
        }
    }

    // BRANCH SENT
    if (branchSent.request != MPI_REQUEST_NULL)
    {
        int isSentFinished;
        MPI_Status status;
        MPI_Test(&branchSent.request, &isSentFinished, &status);
        if (isSentFinished)
        {
            dataManager.sentFinished(branchSent.buffer, branchSent.numElement);
            branchSent.request = MPI_REQUEST_NULL;
        }
    }

    // BOUND
    int isBoundIncoming;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, BOUND, masterpoolComm, &isBoundIncoming, &status);
    while (isBoundIncoming)
    {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, MPI_STATUS_IGNORE);
        BranchBoundResultSolution *result = dataManager.getSolutionFromBound(buffer);
        this->bound = std::max(this->bound, (double) result->getSolutionResult());
        callback(result);
        MPI_Iprobe(MPI_ANY_SOURCE, BOUND, masterpoolComm, &isBoundIncoming, &status);
    }
}

void MasterpoolManager::epilogue(std::function<const Branch *()> callback)
{
    if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
        return;

    if (branchSent.request == MPI_REQUEST_NULL)
    {
        const Branch *branch = callback();
        if (branch != nullptr)
        {
            std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branch);
            branchSent.buffer = pairToSend.first;
            branchSent.numElement = pairToSend.second;
            MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, masterpoolComm, &branchSent.request);
            tokenTermination.nodeColor = nodeBlack;
        }
    }
}

void MasterpoolManager::sendBound(BranchBoundResultSolution *bound)
{
    this->bound = std::max(this->bound, (double) bound->getSolutionResult());
    if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
        return;

    std::pair<void *, int> pairToSend = dataManager.getBoundBuffer(bound);
    MPI_Send(pairToSend.first, pairToSend.second, dataManager.getBoundType(), nextRankToSend, BOUND, masterpoolComm);
}

void MasterpoolManager::sendToken()
{
    if (!tokenTermination.hasToken)
        return;
    tokenTermination.tokenColor = masterpoolRank == 0 ? tokenWhite : tokenTermination.tokenColor;
    int rankToSend = (masterpoolRank + 1) % masterpoolSize;
    // cout << "send token to " << rankToSend << endl;
    if (tokenTermination.nodeColor == nodeWhite)
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, rankToSend, TOKEN, masterpoolComm);
    else
    {
        tokenTermination.tokenColor = tokenBlack;
        MPI_Send(&tokenTermination.tokenColor, 1, MPI_INT, rankToSend, TOKEN, masterpoolComm);
    }
    tokenTermination.nodeColor = nodeWhite;
    tokenTermination.hasToken = false;
}

double MasterpoolManager::getBound() {
    return bound;
}

BranchBoundResultBranch *MasterpoolManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, &status);
    BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    return branch;
}

bool MasterpoolManager::isCommEnabled() {
    return masterpoolComm != MPI_COMM_NULL;
}

void MasterpoolManager::broadcastTerminationWithValue(bool value) {

}