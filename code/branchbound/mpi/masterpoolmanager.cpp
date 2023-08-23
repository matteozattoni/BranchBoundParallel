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
    if (masterpoolComm != MPI_COMM_NULL)
    {
        MPI_Comm_size(masterpoolComm, &masterpoolSize);
        MPI_Comm_rank(masterpoolComm, &masterpoolRank);
        nextRankToSend = (masterpoolRank + 1) % masterpoolSize;
        tokenTermination.hasToken = masterpoolRank == 0 ? true : false;
    }
    MPI_Group_free(&masterGroup);
    std::cout << "World rank: " << worldRank;
    if (masterpoolComm == MPI_COMM_NULL)
    {
        std::cout << " - I'm NOT in master pool" << std::endl;
    }
    else
        std::cout << " - I'm IN master pool! with rank: " << masterpoolRank << std::endl;
    workpoolManager = new WorkpoolManager(manager);
}

MasterpoolManager::~MasterpoolManager()
{
    if (masterpoolComm != MPI_COMM_NULL)
    {
        checkTermination();
        MPI_Comm_free(&masterpoolComm);
    }
    delete workpoolManager;
}

BranchBoundProblem *MasterpoolManager::getBranchProblem()
{
    throw MPIUnimplementedException("MPIGlobalManager::getBranchProblem()");
}

Branch *MasterpoolManager::getRootBranch()
{
    throw MPIUnimplementedException("MPIGlobalManager::getRootBranch()");
}

BranchBoundResultBranch *MasterpoolManager::getBranch()
{
    try
    {
        return workpoolManager->getBranch();
    }
    catch (const MPILocalTerminationException &e)
    {

        if (masterpoolComm == MPI_COMM_NULL && masterpoolSize > 1)
            throw MPILocalTerminationException();

        if (branchReceived.request != MPI_REQUEST_NULL && masterpoolSize > 1)
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

        if (branchReceived.request == MPI_REQUEST_NULL && masterpoolSize > 1)
        {
            int isRecvIncoming;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, masterpoolComm, &isRecvIncoming, &status);
            if (isRecvIncoming)
                return returnBranchFromStatus(status);
        }

        throw MPILocalTerminationException();
    }
}

BranchBoundResultBranch *MasterpoolManager::waitForBranch()
{
    try
    {
        return getBranch();
    }
    catch (const MPILocalTerminationException &e)
    {

        try
        {
            return workpoolManager->waitForBranch();
        }
        catch (const MPIGlobalTerminationException &e)
        {
            if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
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

            if (tokenTermination.hasToken && isLocalTerminate())
                sendToken();

            MPI_Status status;
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, masterpoolComm, &status);
            return getResultFromStatus(status);
        }
    }
    catch (const std::exception &e)
    {
        throw e;
    }
    throw MPIGeneralException("MasterpoolManager::waitForBranch - the last code line should'nt be reach");
}

void MasterpoolManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    workpoolManager->prologue(callback);

    if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
        return;

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
        MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, masterpoolComm, &isBranchIncoming, &status);
        if (isBranchIncoming)
        {
            int countElem;
            MPI_Get_count(&status, dataManager.getBranchType(), &countElem);
            if (countElem == MPI_UNDEFINED)
                throw MPIGeneralException("MasterpoolManager::prologue - MPI_Get_count return MPI_UNDEFINED");
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
        callback(result);
        MPI_Iprobe(MPI_ANY_SOURCE, BOUND, masterpoolComm, &isBoundIncoming, &status);
    }
}

void MasterpoolManager::epilogue(std::function<const Branch *()> callback)
{
    workpoolManager->epilogue(callback);
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
            totalSendBranch++;
            MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, masterpoolComm, &branchSent.request);
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
                MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, BRANCH, masterpoolComm, &branchSent.request);
                tokenTermination.nodeColor = nodeBlack;
            }
        }
    }
}

void MasterpoolManager::sendBound(BranchBoundResultSolution *bound)
{
    workpoolManager->sendBound(bound);
    if (this->bound >= bound->getSolutionResult())
        return;

    this->bound = bound->getSolutionResult();
    
    if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
        return;

    std::pair<void *, int> pairToSend = dataManager.getBoundBuffer(bound);
    MPI_Send(pairToSend.first, pairToSend.second, dataManager.getBoundType(), nextRankToSend, BOUND, masterpoolComm);
}

void MasterpoolManager::sendToken()
{
    if (!tokenTermination.hasToken)
        return;


    tokenTermination.tokenColor = masterpoolRank == 0 && tokenTermination.nodeColor == nodeWhite ? tokenWhite : tokenTermination.tokenColor;

/*     if (branchReceived.request != MPI_REQUEST_NULL || branchSent.request != MPI_REQUEST_NULL)
        tokenTermination.nodeColor = nodeBlack; */

/*     int isBranchIncoming;
    MPI_Iprobe(MPI_ANY_SOURCE, BRANCH, masterpoolComm, &isBranchIncoming, MPI_STATUS_IGNORE);
    if (isBranchIncoming)
        tokenTermination.nodeColor = nodeBlack; */

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

double MasterpoolManager::getBound()
{
    return bound;
}

BranchBoundResultBranch *MasterpoolManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    if (count == MPI_UNDEFINED)
        throw MPIGeneralException("MasterpoolManager::returnBranchFromStatus get_count return MPI_UNDEFINED");
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, &status);
    BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    totalRecvBranch++;
    return branch;
}

bool MasterpoolManager::isCommEnabled()
{
    return masterpoolComm != MPI_COMM_NULL;
}

void MasterpoolManager::broadcastTerminationWithValue(bool value)
{
}

void MasterpoolManager::checkTermination()
{
    int someMessage;
    MPI_Status status;
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, masterpoolComm, &someMessage, &status);
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

BranchBoundResultBranch *MasterpoolManager::getResultFromStatus(MPI_Status status)
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
            {                                            // global termination
                if (!isLocalTerminate())
                    throw MPIGeneralException("Recv Termination before local termination!!");
                for (int i = 1; i < masterpoolSize; i++) // ignore the first root
                {
                    int termination = 1;
                    MPI_Send(&termination, 1, MPI_INT, i, TERMINATION, masterpoolComm);
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
        if (!isLocalTerminate())
            std::cout << "THIS SHOULDNT HAPPEN" << std::endl;
        int termination;
        MPI_Recv(&termination, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, MPI_STATUS_IGNORE);
        throw MPIGlobalTerminationException();
        break;
    }
    case BOUND:
    {
        void *buffer = dataManager.getEmptybBoundBuff();
        MPI_Recv(buffer, 1, dataManager.getBoundType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, MPI_STATUS_IGNORE);
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

bool MasterpoolManager::isLocalTerminate() {
    if (branchReceived.request != MPI_REQUEST_NULL)
        return false;
/*     if (branchSent.request != MPI_REQUEST_NULL)
    {
        std::cout << "branchSent request is not null! Mastermanager " << std::endl;
    } */
    return true;
}

void MasterpoolManager::terminate() {
    workpoolManager->terminate();

    if (masterpoolComm == MPI_COMM_NULL)
        return;

    long totalRecv;
    long totalSend;

    MPI_Reduce(&totalRecvBranch, &totalRecv, 1, MPI_LONG, MPI_SUM, 0, masterpoolComm);
    MPI_Reduce(&totalSendBranch, &totalSend, 1, MPI_LONG, MPI_SUM, 0, masterpoolComm);

    if (masterpoolRank == 0) {
        std::cout << "MASTERPOOL * Total send are: " << totalSend << std::endl;
        std::cout << "MASTERPOOL * Total recv are: " << totalRecv << std::endl;
    }
    
}

