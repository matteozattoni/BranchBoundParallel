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
        previousRankToReceive = masterpoolRank == 0 ? masterpoolSize - 1 : masterpoolRank - 1;
        tokenTermination.hasToken = masterpoolRank == 0 ? true : false;
        treeBranchSent[0].child = (2 * masterpoolRank) + 1;
        treeBranchSent[0].mustBeSent = treeBranchSent[0].child < (masterpoolSize);
        treeBranchSent[1].child = (2 * masterpoolRank) + 2;
        treeBranchSent[1].mustBeSent = treeBranchSent[1].child < (masterpoolSize);
    }
    MPI_Group_free(&masterGroup);
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

        if (masterpoolComm == MPI_COMM_NULL)
            throw MPILocalTerminationException();

        std::list<Branch *> branches;

        if (ringBranchReceived.request != MPI_REQUEST_NULL && masterpoolSize > 1)
        {
            int isRecvFinished;
            MPI_Status status;
            MPI_Test(&ringBranchReceived.request, &isRecvFinished, &status);
            if (isRecvFinished)
            {
                Branch *branch = dataManager.getBranchFromBuff(ringBranchReceived.buffer, ringBranchReceived.numElement);
                branches.push_front(branch);
                totalRecvBranch++;
                ringBranchReceived.request = MPI_REQUEST_NULL;
            }
        }

        if (ringBranchReceived.request == MPI_REQUEST_NULL && masterpoolSize > 1)
        {
            int isRecvIncoming;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, RING_BRANCH, masterpoolComm, &isRecvIncoming, &status);
            if (isRecvIncoming)
            {
                Branch *branch = returnBranchFromStatus(status);
                branches.push_front(branch);
            }
        }

        if (treeBranchReceive[0].request != MPI_REQUEST_NULL)
        {
            MPI_Status status;
            MPI_Wait(&treeBranchReceive[0].request, &status);
            Branch *branch = dataManager.getBranchFromBuff(treeBranchReceive[0].buffer, treeBranchReceive[0].numElement);
            branches.push_front(branch);
            totalRecvBranch++;
            treeBranchReceive[0].request = MPI_REQUEST_NULL;
        }

        if (treeBranchReceive[1].request != MPI_REQUEST_NULL)
        {
            MPI_Status status;
            MPI_Wait(&treeBranchReceive[1].request, &status);
            Branch *branch = dataManager.getBranchFromBuff(treeBranchReceive[1].buffer, treeBranchReceive[1].numElement);
            branches.push_front(branch);
            totalRecvBranch++;
            treeBranchReceive[1].request = MPI_REQUEST_NULL;
        }

        if (branches.size() > 0)
        {
            BranchBoundResultBranch *result = dataManager.getBranchResultFromBranches(branches);
            return result;
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
                if (masterpoolRank == 0 && tokenTermination.nodeColor == nodeWhite && tokenTermination.tokenColor == tokenWhite)
                    throw MPIGlobalTerminationException();
                sendToken();
            }
            Branch *b = nullptr;
            while (b == nullptr)
            {
                MPI_Status status;
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, masterpoolComm, &status);
                b = getBranchFromGeneralStatus(status);
            }
            BranchBoundResultBranch *result = dataManager.getBranchResultFromBranches({b});
            return result;
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

    if (cacheLastBoundMessage != nullptr)
    {
        callback(cacheLastBoundMessage);
        cacheLastBoundMessage = nullptr;
    }

    // BRANCH RECEIVED
    if (ringBranchReceived.request == MPI_REQUEST_NULL)
    {
        int isBranchIncoming;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, RING_BRANCH, masterpoolComm, &isBranchIncoming, &status);
        if (isBranchIncoming)
        {
            int countElem;
            MPI_Get_count(&status, dataManager.getBranchType(), &countElem);
            if (countElem == MPI_UNDEFINED)
                throw MPIGeneralException("MasterpoolManager::prologue - MPI_Get_count return MPI_UNDEFINED");
            ringBranchReceived.buffer = dataManager.getEmptyBranchElementBuff(countElem);
            ringBranchReceived.numElement = countElem;
            MPI_Irecv(ringBranchReceived.buffer, countElem, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, &ringBranchReceived.request);
        }
    }

    for (int i = 0; i < 2; i++)
    {
        if (treeBranchReceive[i].canBeReceive && treeBranchReceive[i].request == MPI_REQUEST_NULL)
        {
            int isBranchIncoming;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, TREE_BRANCH, masterpoolComm, &isBranchIncoming, &status);
            if (isBranchIncoming)
            {
                int countElem;
                MPI_Get_count(&status, dataManager.getBranchType(), &countElem);
                if (countElem == MPI_UNDEFINED)
                    throw MPIGeneralException("MasterpoolManager::prologue - MPI_Get_count return MPI_UNDEFINED");
                treeBranchReceive[i].buffer = dataManager.getEmptyBranchElementBuff(countElem);
                treeBranchReceive[i].numElement = countElem;
                MPI_Irecv(treeBranchReceive[i].buffer, countElem, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, &treeBranchReceive[i].request);
            }
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

    if (masterpoolComm == MPI_COMM_NULL || masterpoolSize < 2)
    {
        workpoolManager->epilogue(callback);
        return;
    }

    for (int i = 0; i < 2; i++)
    {
        if (treeBranchSent[i].mustBeSent)
        {
            if (treeBranchSent[i].request == MPI_REQUEST_NULL)
            {
                const Branch *branchToSend = callback();
                if (branchToSend != nullptr)
                {
                    std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branchToSend);
                    treeBranchSent[i].buffer = pairToSend.first;
                    treeBranchSent[i].numElement = pairToSend.second;
                    MPI_Issend(pairToSend.first, pairToSend.second, dataManager.getBranchType(), treeBranchSent[i].child, TREE_BRANCH, masterpoolComm, &treeBranchSent[i].request);
                    totalSendBranch++;
                    tokenTermination.nodeColor = nodeBlack;
                }
            }
            else
            {
                int isSentFinished;
                MPI_Test(&treeBranchSent[i].request, &isSentFinished, MPI_STATUS_IGNORE);
                if (isSentFinished)
                {
                    dataManager.sentFinished(treeBranchSent[i].buffer, treeBranchSent[i].numElement);
                    treeBranchSent[i].request = MPI_REQUEST_NULL;
                    const Branch *branch = callback();
                    if (branch != nullptr)
                    {
                        std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branch);
                        treeBranchSent[i].buffer = pairToSend.first;
                        treeBranchSent[i].numElement = pairToSend.second;
                        totalSendBranch++;
                        MPI_Issend(treeBranchSent[i].buffer, treeBranchSent[i].numElement, dataManager.getBranchType(), treeBranchSent[i].child, TREE_BRANCH, masterpoolComm, &treeBranchSent[i].request);
                        tokenTermination.nodeColor = nodeBlack;
                    }
                }
            }
        }
    }

    if (branchSent.request == MPI_REQUEST_NULL)
    {
        const Branch *branch = callback();
        if (branch != nullptr)
        {
            std::pair<void *, int> pairToSend = dataManager.getBranchBuffer(branch);
            branchSent.buffer = pairToSend.first;
            branchSent.numElement = pairToSend.second;
            totalSendBranch++;
            MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, RING_BRANCH, masterpoolComm, &branchSent.request);
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
                MPI_Issend(branchSent.buffer, branchSent.numElement, dataManager.getBranchType(), nextRankToSend, RING_BRANCH, masterpoolComm, &branchSent.request);
                tokenTermination.nodeColor = nodeBlack;
            }
        }
    }

    workpoolManager->epilogue(callback);
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

Branch *MasterpoolManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    if (count == MPI_UNDEFINED)
        throw MPIGeneralException("MasterpoolManager::returnBranchFromStatus get_count return MPI_UNDEFINED");
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, &status);
    Branch *branch = dataManager.getBranchFromBuff(buffBranch, count);
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
    if (someMessage && status.MPI_TAG == RING_BRANCH)
        std::cout << "There is some branch message before deallocating Mastermanager" << std::endl;
    if (ringBranchReceived.request != MPI_REQUEST_NULL)
        std::cout << "branchReceived request is not null! Mastermanager " << std::endl;
    if (branchSent.request != MPI_REQUEST_NULL)
    {
        int isFinished;
        MPI_Test(&branchSent.request, &isFinished, MPI_STATUS_IGNORE);
        if (!isFinished)
            std::cout << "branchSent request has not been read! Mastermanager " << std::endl;
    }
}

Branch *MasterpoolManager::getBranchFromGeneralStatus(MPI_Status status)
{
    switch (status.MPI_TAG)
    {
    case TREE_BRANCH:
    case RING_BRANCH:
    {
        return returnBranchFromStatus(status);
        break;
    }
    case TOKEN:
    {
        int isBranchIncoming;
        int isBranchReceived;
        MPI_Status branchStatus;
        if (ringBranchReceived.request != MPI_REQUEST_NULL)
        {
            MPI_Test(&ringBranchReceived.request, &isBranchReceived, &branchStatus);
            Branch *branch = dataManager.getBranchFromBuff(ringBranchReceived.buffer, ringBranchReceived.numElement);
            totalRecvBranch++;
            ringBranchReceived.request = MPI_REQUEST_NULL;
            return branch;
        }

        MPI_Iprobe(MPI_ANY_SOURCE, RING_BRANCH, masterpoolComm, &isBranchIncoming, &branchStatus);
        if (isBranchIncoming)
        { // there is some branch
            return returnBranchFromStatus(branchStatus);
        }
        else
        { // nothing incoming we take the token
            MPI_Recv(&tokenTermination.tokenColor, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, masterpoolComm, MPI_STATUS_IGNORE);
            tokenTermination.hasToken = true;
            if (masterpoolRank == 0 && tokenTermination.tokenColor == tokenWhite)
            { // global termination
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
                return nullptr;
            }
        }
        break;
    }
    case TERMINATION:
    {
        /*         if (!isLocalTerminate())
                    std::cout << "THIS SHOULDNT HAPPEN" << std::endl; */
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
        if (bound > result->getSolutionResult())
            delete result;
        else
        {
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
        throw MPIGeneralException("MasterpoolManager::waitForBranch - Unknown tag ");
        break;
    }
    }
}

bool MasterpoolManager::isLocalTerminate()
{
    if (ringBranchReceived.request != MPI_REQUEST_NULL)
        return false;
    /*     if (branchSent.request != MPI_REQUEST_NULL)
        {
            std::cout << "branchSent request is not null! Mastermanager " << std::endl;
        } */
    return true;
}

void MasterpoolManager::terminate()
{
    workpoolManager->terminate();

    if (masterpoolComm == MPI_COMM_NULL)
        return;

    long totalRecv;
    long totalSend;

    MPI_Reduce(&totalRecvBranch, &totalRecv, 1, MPI_LONG, MPI_SUM, 0, masterpoolComm);
    MPI_Reduce(&totalSendBranch, &totalSend, 1, MPI_LONG, MPI_SUM, 0, masterpoolComm);

    if (masterpoolRank == 0)
    {
        std::cout << "MASTERPOOL (size " << masterpoolSize << ") * Total send: " << totalSend << " - recv: " << totalRecv << std::endl;
    }
}
