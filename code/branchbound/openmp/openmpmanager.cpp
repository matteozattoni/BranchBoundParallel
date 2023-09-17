
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
    omp_set_num_threads(NUM_THREADS_HELPER + NUM_THREADS_WORKER);
#pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
#pragma omp masked
        {
            nextManager = new MPIBranchBoundManager(dataManager);
            std::cout << "Comm rank: " << nextManager->getIdentity() << " | " << num_threads << " threads (worker: " << NUM_THREADS_WORKER << " - helper: " << NUM_THREADS_HELPER << ")" << std::endl;
            problem = nextManager->getBranchProblem();
        }

#pragma omp barrier
        if (thread_id < NUM_THREADS_WORKER)
        {
            BranchBound *branchBound = new BranchBound(this, call());
            threadsData[thread_id].thread_id = thread_id;
            threadsData[thread_id].orchestrator = branchBound;
            omp_init_lock(&threadsData[thread_id].lockBound);
            omp_init_lock(&threadsData[thread_id].lockList);
            try
            {
                branchBound->start();
                throw End;
                if (thread_id == 0)
                    delete nextManager;
            }
            catch (const eBranchBoundException e)
            {
                std::cout << "Branch Exc: " << e << std::endl;
            }
            catch (const MPIBranchBoundTerminationException &es)
            {
                if (thread_id == 0)
                {
                    std::cout << "Final solution is " << es.finalSolution << std::endl;
                    delete nextManager;
                }
            }
            catch (const int i)
            {
            }
            catch (const std::exception &x)
            {
                std::cout << x.what() << std::endl;
            }
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

    if (!threadsData[thread_id].cachedBranch.empty())
    {
        Branch *branch = (Branch *)threadsData[thread_id].cachedBranch.front();
        threadsData[thread_id].cachedBranch.pop_front();
        result = dataManager.getBranchResultFromBranches({branch});
        return result;
    }

    omp_set_lock(&threadsData[thread_id].lockList);
    if (threadsData[thread_id].branches.size() > 0)
    {
        Branch *branch = (Branch *)threadsData[thread_id].branches.front();
        threadsData[thread_id].branches.pop_front();
        result = dataManager.getBranchResultFromBranches({branch});
    }
    omp_unset_lock(&threadsData[thread_id].lockList);

    if (result != nullptr)
    {
        return result;
    }

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
                if (numOfThreadWaitingForBranch == NUM_THREADS_WORKER)
                {
                    try
                    {
                        bool allAreWaiting = true;
#pragma omp taskwait
                        for (int i = 0; i < NUM_THREADS_WORKER; i++)
                        {
                            omp_set_lock(&threadsData[i].lockList);
                            allAreWaiting = allAreWaiting && (threadsData[i].branches.size() == 0);
                            omp_unset_lock(&threadsData[i].lockList);
                        }
                        if (allAreWaiting)
                            result = nextManager->waitForBranch();
                    }
                    catch (const MPIBranchBoundTerminationException &e)
                    {
                        globalTermination = true;
                    }
                }
            }
        }

        if (globalTermination)
            break;

        if (result != nullptr)
            break;

        for (int i = 0; i < NUM_THREADS_WORKER && result == nullptr; i++)
        {
            if (i != thread_id)
            {
                omp_set_lock(&threadsData[i].lockList);
                if (threadsData[i].branches.size() > 0)
                {
                    Branch *branch = (Branch *)threadsData[i].branches.front();
                    threadsData[i].branches.pop_front();
                    result = dataManager.getBranchResultFromBranches({branch});
                }
                omp_unset_lock(&threadsData[i].lockList);
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

    const Branch *branch = callback();
    while (branch != nullptr)
    {
        threadsData[thread_id].cachedBranch.push_front(branch);
        branch = callback();
    }

    std::list<const Branch *> newBranches;

    if (threadsData[thread_id].cachedBranch.size() > 3)
    {
        auto it = threadsData[thread_id].cachedBranch.begin();
        std::advance(it, 2);
        newBranches.splice(newBranches.end(), threadsData[thread_id].cachedBranch, it);
    }

    if (newBranches.size() > 0)
    {
#pragma omp task private(newBranches) firstprivate(thread_id)
        {
            while(!omp_test_lock(&threadsData[thread_id].lockList)) {
                #pragma omp taskyield
            }
            threadsData[thread_id].branches.insert(threadsData[thread_id].branches.end(), newBranches.begin(), newBranches.end());
            if (thread_id == 0)
                nextManager->epilogue([this, thread_id]() -> const Branch *
                                      {
                    if (threadsData[thread_id].branches.size() > 0) {
                        const Branch* branch = threadsData[thread_id].branches.front();
                        threadsData[thread_id].branches.pop_front();
                        return branch;
                    }
                    return nullptr; });
            omp_unset_lock(&threadsData[thread_id].lockList);
        }
    }
}

void OpenMPManager::sendBound(BranchBoundResultSolution *bound)
{
    const int myId = omp_get_thread_num();
    const double solution = bound->getSolutionResult();
#pragma omp task firstprivate(myId, solution)
    {
        for (int i = 0; i < NUM_THREADS_WORKER; i++)
        {
            if (i != myId)
            {
                while(!omp_test_lock(&threadsData[i].lockBound)) {
                    #pragma omp taskyield
                }
                if (solution > threadsData[i].bound)
                    threadsData[i].bound = solution;
                omp_unset_lock(&threadsData[i].lockBound);
            }
        }
    }
}