
#include <iostream>
#include <optional>
#include "omp.h"
#include "openmpmanager.h"
#include "../branchbound.h"
#include "../algorithm/branchboundexception.h"
#include "../mpi/mpibranchboundmanager.h"

OpenMPManager::OpenMPManager(MPIDataManager &mpiDataManager) : dataManager(mpiDataManager)
{
}

OpenMPManager::~OpenMPManager()
{
}

void OpenMPManager::start(std::function<BranchBoundAlgorithm *()> call)
{
    // set number of threads
    // omp_set_dynamic(1);
    omp_set_num_threads(NUM_THREADS);
#pragma omp parallel private(thread_id)
    {
        thread_id = omp_get_thread_num();
        std::cout << " i am " << thread_id << std::endl;
        BranchBound *branchBound = new BranchBound(this, call());

#pragma omp master
        {
            std::cout << " master: " << thread_id << std::endl;
            nextManager = new MPIBranchBoundManager(dataManager);
            problem = nextManager->getBranchProblem();
        }

#pragma omp barrier

        try
        {
            branchBound->start();
            if (thread_id == 0)
                delete nextManager;
        }
        catch (const eBranchBoundException e)
        {
            std::cout << "Branch Exc: " << e << std::endl;
        }
        catch (const MPIBranchBoundTerminationException &es)
        {
            std::cout << "finish start: " << es.finalSolution << std::endl;
            if (thread_id == 0)
                delete nextManager;
        }
    }
}

BranchBoundProblem *OpenMPManager::getBranchProblem()
{
    return problem;
}

Branch *OpenMPManager::getRootBranch()
{
    if (nextManager != nullptr)
        return nextManager->getRootBranch();
    else
        return nullptr;
}

BranchBoundResultBranch *OpenMPManager::getBranch()
{
    if (nextManager != nullptr)
        return nextManager->getBranch();
    else
        return nullptr;
}

BranchBoundResultBranch *OpenMPManager::waitForBranch()
{
    BranchBoundResultBranch *result = nullptr;
    const int thread_id = omp_get_thread_num();

#pragma omp atomic
    numOfThreadWaitingForBranch++;

    while (result == nullptr)
    {
        if (thread_id == 0)
        {
            try
            {
                result = nextManager->getBranch();
            }
            catch (const MPILocalTerminationException &e)
            {
#pragma omp critical(criticalListBranch)
                {
                    if (listOfBranches.size() <= 0 && numOfThreadWaitingForBranch == NUM_THREADS)
                    {
                        try
                        {
                            result = nextManager->waitForBranch();
                        }
                        catch (const MPIBranchBoundTerminationException &e)
                        {
                            globalTermination = true;
                        }
                    }
                }
            }
        }

#pragma omp critical(criticalListBranch)
        {
            if (listOfBranches.size() > 0)
            {
                Branch *branch = (Branch *)listOfBranches.front();
                listOfBranches.pop_front();
                result = dataManager.getBranchResultFromBranches({branch});
            }
        }

        if (globalTermination)
        {
            std::cout << "term " << thread_id << std::endl;
            throw MPIBranchBoundTerminationException(22);
        }
    }

#pragma omp atomic
    numOfThreadWaitingForBranch--;

    return result;
}

void OpenMPManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    const int thread_id = omp_get_thread_num();
    if (thread_id == 0)
        nextManager->prologue(callback);
}

void OpenMPManager::epilogue(std::function<const Branch *()> callback)
{
    const int thread_id = omp_get_thread_num();
    if (thread_id == 0)
        nextManager->epilogue(callback);

    std::list<const Branch *> newBranches;
    const Branch *branch = callback();
    while (branch != nullptr)
    {
        newBranches.push_front(branch);
        branch = callback();
    }

    if (newBranches.size() > 0)
    {
#pragma omp critical(criticalListBranch)
        {
            listOfBranches.insert(listOfBranches.end(), newBranches.begin(), newBranches.end());
            if (thread_id == 0)
                nextManager->epilogue([this] -> const Branch *
                                      {
                    if (listOfBranches.size() > 0) {
                        const Branch* branch = listOfBranches.front();
                        listOfBranches.pop_front();
                        return branch;
                    }
                    return nullptr; });
        }
    }
}

void OpenMPManager::sendBound(BranchBoundResultSolution *bound)
{
    std::cout << "new bound " << bound->getSolutionResult() << std::endl;
}