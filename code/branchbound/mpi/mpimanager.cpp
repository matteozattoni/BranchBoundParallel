#include "mpimanager.h"
#include <iostream>
#include <cmath>

using namespace std;

MPIManager::MPIManager(MPIDataManager *manager) : dataManager(*manager)
{
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    const int myworkpool = worldRank / WORKPOOL_N;
    MPI_Comm_split(MPI_COMM_WORLD, myworkpool, worldRank, &workpoolComm);
    MPI_Comm_size(workpoolComm, &workpoolSize);
    MPI_Comm_rank(workpoolComm, &workpoolRank);
    receiveBranch[workpoolRank].ready = false;
    sentBranch[workpoolRank].isSent = true;
    manager->commitDatatypes();
    receiveBound.boundBuffer = manager->getEmptybBoundBuff();
    MPI_Irecv(receiveBound.boundBuffer, 1, manager->getBoundType(), MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &receiveBound.request);
    if (workpoolRank == 0)
        tokenTermination.hasToken = true;
}

MPIManager::~MPIManager()
{
    MPI_Finalize();
}

const Branch *MPIManager::getRootBranch()
{
    if (worldRank == 0)
        return dataManager.getRootBranch();
    else
        return nullptr;
}

void MPIManager::init()
{
}

BranchBoundProblem *MPIManager::getBranchProblem()
{
    MPI_Datatype problemType = dataManager.getProblemType();
    MPI_Datatype problemElementType = dataManager.getProblemElementType();
    void *problemDescritpionBuffer;
    std::pair<void *, int> problemElementsPair;
    if (worldRank == MASTER_RANK)
    {
        BranchBoundProblem *localProblem = dataManager.getLocalProblem();
        problemDescritpionBuffer = dataManager.getProblemTypeBuffFrom(localProblem);
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getProblemElementBuffFrom(localProblem);
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, MASTER_RANK, MPI_COMM_WORLD);
        return localProblem; // could be memory leak
    }
    else
    {
        problemDescritpionBuffer = dataManager.getEmptyProblemTypeBuff();
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getEmptyProblemElementBuffFromType(problemDescritpionBuffer);
        MPI_Status status;
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, MASTER_RANK, MPI_COMM_WORLD);
        BranchBoundProblem *remoteProblem = dataManager.getRemoteProblem(problemDescritpionBuffer, problemElementsPair);
        return remoteProblem; // could be memory leak
    }
}

void MPIManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    int boundFlag;
    MPI_Status status;

    // get bound messages if any
    receiveBoundMessage(callback);

    // load balance protocol
    loadBalance(callback);
}

void MPIManager::receiveBoundMessage(std::function<void(BranchBoundResult *)> callback) {
    int isBoundMessageArrived;
    MPI_Status status;
    MPI_Test(&(receiveBound.request), &isBoundMessageArrived, &status);
    if (isBoundMessageArrived)
    {
        BranchBoundResultSolution *sol = dataManager.getSolutionFromBound(receiveBound.boundBuffer);
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
                callback(sol);
                receiveBound.boundBuffer = nullptr;
            }
        }
        receiveBound.boundBuffer = dataManager.getEmptybBoundBuff();
        MPI_Irecv(receiveBound.boundBuffer, 1, dataManager.getBoundType(), MPI_ANY_SOURCE, TAG_BOUND, workpoolComm, &(receiveBound.request));
    }
}

void MPIManager::loadBalance(std::function<void(BranchBoundResult *)> callback)
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

void MPIManager::epilogue(std::function<const Branch *()> callback)
{
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

void MPIManager::sendBound(BranchBoundResultSolution *bound)
{
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

BranchBoundResultBranch *MPIManager::waitForBranch()
{
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
    { // all receive are not commited so i'll wait the first message from other (termination)
        return terminationProtocol();
    }
}

// PRIVATE

// local termination (no branch/no receive)
BranchBoundResultBranch *MPIManager::terminationProtocol()
{
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
                throw TERMINATED;
            sendToken(tokenTermination.tokenColor);
            return waitForBranch();
        }
    }
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, workpoolComm, &status);
    if (status.MPI_TAG == TAG_BRANCH)
    {
        return returnBranchFromStatus(status);
    }
    else if (status.MPI_TAG == TAG_TOKEN)
    {
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
                for (int i = 1; i < workpoolSize; i++) // ignore the first
                {
                    int termination;
                    MPI_Send(&termination, 1, MPI_INT, i, TAG_TERMINATION, workpoolComm);
                }
                throw TERMINATED;
            }
            else
            {
                sendToken(tokenTermination.tokenColor);
                return waitForBranch();
            }
        }
    }
    else if (status.MPI_TAG == TAG_TERMINATION)
    {
        int termination;
        MPI_Recv(&termination, 1, MPI_INT, 0, TAG_TERMINATION, workpoolComm, MPI_STATUS_IGNORE);
        throw TERMINATED;
    }
    throw 55;
}

void MPIManager::sendToken(eTokenColor sendWithThisColor)
{
    if (!tokenTermination.hasToken)
        return;
    tokenTermination.tokenColor = workpoolRank == 0 ? tokenWhite : sendWithThisColor;
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

BranchBoundResultBranch *MPIManager::returnBranchFromStatus(MPI_Status status)
{
    int count;
    MPI_Get_count(&status, dataManager.getBranchType(), &count);
    void *buffBranch = dataManager.getEmptyBranchElementBuff(count);
    MPI_Recv(buffBranch, count, dataManager.getBranchType(), status.MPI_SOURCE, status.MPI_TAG, workpoolComm, &status);
    BranchBoundResultBranch *branch = dataManager.getBranchFromBuff(buffBranch, count);
    return branch;
}

void MPIManager::defineType()
{
    struct
    {
        int a;
        double b;
        double c;
    } myss;

    int size;
    MPI_Aint lb, ext;
    MPI_Datatype mystruct;

    int arrayOfBlocklenghs[] = {1, 2};
    MPI_Aint arrayOfDisplacements[2];
    MPI_Datatype arrayOfOldDatatype[] = {MPI_INT, MPI_DOUBLE};
    MPI_Get_address(&myss.a, &arrayOfDisplacements[0]);
    MPI_Get_address(&myss.b, &arrayOfDisplacements[1]);
    arrayOfDisplacements[1] -= arrayOfDisplacements[0];
    cout << "disp is " << arrayOfDisplacements[1] << endl;
    arrayOfDisplacements[0] = 0;
    MPI_Type_create_struct(2, arrayOfBlocklenghs, arrayOfDisplacements, arrayOfOldDatatype, &mystruct);
    MPI_Type_commit(&mystruct);
    MPI_Type_size(mystruct, &size);
    MPI_Type_get_extent(mystruct, &lb, &ext);
    cout << "mpi size is " << size << " lb: " << lb << " ext: " << ext << endl;
    cout << "struct size is " << sizeof(myss) << endl;
}