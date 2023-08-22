#include "branchbound.h"
#include "branchboundmemorymanager.h"
#include "algorithm/branchboundexception.h"
#include "mpi/mpiexceptions.h"
#include <iostream>

int BranchBound::rank = -1;

BranchBound::BranchBound(MPIBranchBoundManager *mpiManager, BranchBoundAlgorithm *algorithm) : worldRank(mpiManager->getWorldRank())
{
    rank = mpiManager->getWorldRank();
    this->mpiManager = mpiManager;
    this->algorithm = algorithm;
}

BranchBound::~BranchBound() {}

void BranchBound::start()
{
    BranchBoundProblem *problem;

    problem = mpiManager->getBranchProblem();
    // get problem from mpi

    algorithm->setProblem(problem);

    if (firstExecution)
    {
        Branch *rootBranch = mpiManager->getRootBranch();
        if (rootBranch != nullptr)
            algorithm->setBranch(rootBranch);
        firstExecution = false;
    }

    while (true)
    {
        // get bound from mpi if any
        while (!algorithm->hasCurrentBranch())
        { // check if algorith has current task, if dont wait for one or extract it from list
            Branch *branch = getTaskFromQueue();
            if (branch != nullptr)
            { // fetched from queue
                
                algorithm->setBranch(branch);
                // now safe to delete (has been copied)
                delete branch;
            }
            else
            {
                // get task from mpi: wait
                try
                {        
                    BranchBoundResultBranch *resultBranch = mpiManager->waitForBranch();
                    Branch *array = resultBranch->getArrayBranch();
                    for (int i = 0; i < resultBranch->getNumberBranch(); i++)
                    {
                        Branch *branch = &array[i];
                        if (i == 0)
                            algorithm->setBranch(branch);
                        else
                            addBranchToQueue(branch);
                    }
                    delete resultBranch;
                }
                catch (const MPIBranchBoundTerminationException& e)
                {
                    if (e.finalSolution > bound) {
                        if (BranchBound::rank == 0)
                            throw e;
                        else
                            throw 0;
                    } else {
                        if (BranchBound::rank == 0)
                            throw MPIBranchBoundTerminationException(bound);
                        else
                            throw 0;
                    }
                        
                    
                    throw e;
                }
            }
        }
        // call prologue from mpi
        mpiManager->prologue(
            [this](BranchBoundResult *result)
            { 
                this->newBranchBoundResult(result);
                /* BranchBoundResultSolution *resultSolution = dynamic_cast<BranchBoundResultSolution *>(result);
                if (resultSolution != nullptr) {
                    int solution = resultSolution->getSolutionResult();
                    // std::cout << "(from rank " << rank << ") sol is " << solution << std::endl;
                    if (algorithm->isBetterBound(solution))
                        {
                        setBound(solution);
                        }
                        delete resultSolution;
                } else
                    this->newBranchBoundResult(result);  */});

        try
        {
            BranchBoundResult *result = algorithm->computeTaskIteration();
            newBranchBoundResult(result);

            // call epilogue from mpi
            mpiManager->epilogue([this]()
                                 {
            const Branch *s = this->getTaskFromQueue();
            return s; });
        }
        catch (eBranchBoundException e)
        {
            std::cerr << e << '\n';
        }
    }
}

void BranchBound::newBranchBoundResult(BranchBoundResult *result)
{
    eBranchBoundResultType resultType = result->getResultType();
    switch (resultType)
    {
    case Solution:
    {
        BranchBoundResultSolution *resultSolution = static_cast<BranchBoundResultSolution *>(result);
        int solution = resultSolution->getSolutionResult();
        // std::cout << "(from rank " << rank << ") sol is " << solution << std::endl;
        if (algorithm->isBetterBound(solution))
        {
            //std::cout << "new local bound: " << solution  <<std::endl;
            setBound(solution);
            sendBound(resultSolution);
        }
        delete resultSolution;
        break;
    }
    case ResultBranch:
    {
        BranchBoundResultBranch *resultBranch = static_cast<BranchBoundResultBranch *>(result);
        Branch *array = resultBranch->getArrayBranch();
        for (int i = 0; i < resultBranch->getNumberBranch(); i++)
        {
            Branch *branch = &array[i];

            addBranchToQueue(branch);
        }
        delete resultBranch;
        break;
    }
    case Closed:
    {
        BranchBoundResultClosed *resultClose = static_cast<BranchBoundResultClosed *>(result);
        delete resultClose;
        break;
    }
    }
}

void BranchBound::sendBound(BranchBoundResultSolution* solution) {
    mpiManager->sendBound(solution);
}

Branch *BranchBound::getTaskFromQueue()
{
    Branch *branch = nullptr;
    if (list.size() > 0)
    {
        branch = list.front();
        list.pop_front();
    }
    return branch;
}

void BranchBound::addBranchToQueue(Branch *branch)
{
    list.push_front(branch);
}

void BranchBound::setBound(int bound)
{
    if (bound == 9147) {
        std::cout << "("<< rank << ") bound 9147 has been set!!! " << std::endl;
    }
    std::cout << "(" << rank <<  ") new bound " << bound << std::endl;
    
    this->bound = bound;
    this->algorithm->setBound(bound);
}

std::ostream &operator<<(std::ostream &out, const BranchBound &data)
{
    out << "Branch and Bound Recap:" << std::endl;
    data.algorithm->printAlgorithm(out);
    return out;
}