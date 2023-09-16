
#include <iostream>
#include <optional>
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
#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
#pragma omp master
        {
            nextManager = new MPIBranchBoundManager(dataManager);
            std::cout << "Comm rank: " << nextManager->getIdentity() << " - Num of threads: " << num_threads << std::endl;
            problem = nextManager->getBranchProblem();
        }

#pragma omp barrier
        BranchBound *branchBound = new BranchBound(this, call());
        threadsData[thread_id].thread_id = thread_id;
        threadsData[thread_id].orchestrator = branchBound;
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
            if (thread_id == 0) {
                std::cout << "Final solution is " << es.finalSolution << std::endl;
                delete nextManager;
            }
        }
        catch (const int i) {

        }
        catch (const std::exception &x) {
            std::cout << x.what() << std::endl;
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

    while (result == nullptr && !globalTermination)
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
                    if (listOfBranches.size() > 0) {
                        Branch *branch = (Branch *)listOfBranches.front();
                        listOfBranches.pop_front();
                        result = dataManager.getBranchResultFromBranches({branch});
                    } else if (numOfThreadWaitingForBranch == NUM_THREADS)
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

        if (result != nullptr || globalTermination)
            break;

#pragma omp critical(criticalListBranch)
        {
            if (listOfBranches.size() > 0)
            {
                Branch *branch = (Branch *)listOfBranches.front();
                listOfBranches.pop_front();
                result = dataManager.getBranchResultFromBranches({branch});
            }
        }

    }

#pragma omp atomic
    numOfThreadWaitingForBranch--;

    if (globalTermination)
        {
            throw MPIBranchBoundTerminationException(22);
        }

    return result;
}

void OpenMPManager::prologue(std::function<void(BranchBoundResult *)> callback)
{
    const int thread_id = omp_get_thread_num();
    if (thread_id == 0)
        nextManager->prologue(callback);
    
    omp_set_lock(&threadsData[thread_id].lockBound);
    threadsData[thread_id].orchestrator->setBound(threadsData[thread_id].bound);
    omp_unset_lock(&threadsData[thread_id].lockBound);
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
                nextManager->epilogue([this] () -> const Branch *
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
    const int myId = omp_get_thread_num();
    const double solution = bound->getSolutionResult();
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (i != myId) {
            omp_set_lock(&threadsData[i].lockBound);
            if (solution > threadsData[i].bound)
                threadsData[i].bound = solution;
            omp_unset_lock(&threadsData[i].lockBound);
        }
    }
}