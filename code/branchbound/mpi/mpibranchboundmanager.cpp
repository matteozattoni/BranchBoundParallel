#include "mpibranchboundmanager.h"
#include "tokenringmanager.h"
#include <iostream>
#include <cmath>

using namespace std;

MPIBranchBoundManager::MPIBranchBoundManager(MPIDataManager &manager) : MPIManager(manager), dataManager(manager)
{
    auto start = Time::now();
    MPI_Init(NULL, NULL);
    manager.commitDatatypes();
    masterpoolManager = new MasterpoolManager(manager);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
    if (worldRank == 0)
        std::cout << "Comm World size: " << worldSize << std::endl;
    auto finish = Time::now();
    totalComunicationTime = finish - start;
}

MPIBranchBoundManager::~MPIBranchBoundManager()
{
    delete masterpoolManager;
    MPI_Finalize();
}

Branch *MPIBranchBoundManager::getRootBranch()
{
    auto start = Time::now();
    Branch *br;
    if (worldRank == 0)
        br = dataManager.getRootBranch();
    else
        br = nullptr;

    auto finish = Time::now();
    fsec fs = finish - start;
    totalComunicationTime += fs;
    return br;
}

BranchBoundProblem *MPIBranchBoundManager::getBranchProblem()
{
    auto start = Time::now();
    MPI_Datatype problemType = dataManager.getProblemType();
    MPI_Datatype problemElementType = dataManager.getProblemElementType();
    void *problemDescritpionBuffer;
    std::pair<void *, int> problemElementsPair;
    BranchBoundProblem *problem;
    if (worldRank == GLOBAL_MASTER_RANK)
    {
        BranchBoundProblem *localProblem = dataManager.getLocalProblem();
        problemDescritpionBuffer = dataManager.getProblemTypeBuffFrom(localProblem);
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getProblemElementBuffFrom(localProblem);
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        problem = localProblem; // could be memory leak
        auto finish = Time::now();
        fsec fs = finish - start;
        totalComunicationTime += fs;
        return problem;
    }
    else
    {
        problemDescritpionBuffer = dataManager.getEmptyProblemTypeBuff();
        MPI_Bcast(problemDescritpionBuffer, 1, problemType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        problemElementsPair = dataManager.getEmptyProblemElementBuffFromType(problemDescritpionBuffer);
        MPI_Bcast(problemElementsPair.first, problemElementsPair.second, problemElementType, GLOBAL_MASTER_RANK, MPI_COMM_WORLD);
        BranchBoundProblem *remoteProblem = dataManager.getRemoteProblem(problemDescritpionBuffer, problemElementsPair);
        problem = remoteProblem; // could be memory leak
        auto finish = Time::now();
        fsec fs = finish - start;
        totalComunicationTime += fs;
        return problem;
    }
}

void MPIBranchBoundManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    auto start = Time::now();
    masterpoolManager->prologue(callback);
    auto finish = Time::now();
    fsec fs = finish - start;
    totalComunicationTime += fs;
}

void MPIBranchBoundManager::epilogue(std::function<const Branch *()> callback)
{
    auto start = Time::now();
    masterpoolManager->epilogue(callback);
    auto finish = Time::now();
    fsec fs = finish - start;
    totalComunicationTime += fs;
}

void MPIBranchBoundManager::sendBound(BranchBoundResultSolution *bound)
{
    auto start = Time::now();
    masterpoolManager->sendBound(bound);
    auto finish = Time::now();
    fsec fs = finish - start;
    totalComunicationTime += fs;
}

BranchBoundResultBranch *MPIBranchBoundManager::getBranch()
{
    auto start = Time::now();
    BranchBoundResultBranch *resultBranch;
    resultBranch = masterpoolManager->getBranch();
    auto finish = Time::now();
    fsec fs = finish - start;
    totalComunicationTime += fs;
    return resultBranch;
}

BranchBoundResultBranch *MPIBranchBoundManager::waitForBranch()
{
    auto start = Time::now();
    try
    {
        BranchBoundResultBranch *resultBranch;
        resultBranch = masterpoolManager->waitForBranch();
        auto finish = Time::now();
        fsec fs = finish - start;
        totalComunicationTime += fs;
        return resultBranch;
    }
    catch (const MPIGlobalTerminationException &e) // Workpool and Masterpool reached Local Termination
    {
        cout << worldRank << "Termination" << endl;
        // std::cout << "(" << worldRank <<" ) TERMINATED" << std::endl;
        terminate();
        int64_t commTime;
        double finalbound = 0.0;
        double maxbound = masterpoolManager->getBound();
        MPI_Reduce(&maxbound, &finalbound, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        auto finish = Time::now();
        fsec fs = finish - start;
        totalComunicationTime += fs;
        int64_t secondCommTime = std::chrono::duration_cast<sec>(totalComunicationTime).count();
        MPI_Reduce(&secondCommTime, &commTime, 1, MPI_INT64_T, MPI_MAX, 0, MPI_COMM_WORLD);
        if (worldRank == 0)
            cout << "Total Comunication time " << commTime << endl;
        throw MPIBranchBoundTerminationException(finalbound);
    }
    throw MPIGeneralException("MPIBranchBoundManager::waitForBranch");
}

bool MPIBranchBoundManager::isCommEnabled()
{
    return true;
}

void MPIBranchBoundManager::broadcastTerminationWithValue(bool value) {}

double MPIBranchBoundManager::getBound()
{
    return masterpoolManager->getBound();
}

void MPIBranchBoundManager::terminate()
{
    masterpoolManager->terminate();
}