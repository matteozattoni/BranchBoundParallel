#include "branchbound.h"
#include <iostream>

BranchBound::BranchBound(BranchBoundAlgorithm *algorithm)
{
    this->algorithm = algorithm;
}

BranchBound::~BranchBound()
{
}

void BranchBound::start(BranchBoundProblem *problem, bool withInitBranch = false)
{

    if (withInitBranch)
    {
        algorithm->setProblemWithRootBranch(problem);
    }
    else
    {
        algorithm->setProblem(problem); // pass the problem to the algorithm
    }

    // get problem from mpi
    // get initTask from mpi
    while (true)
    {
        if (!algorithm->hasCurrentBranch())
        { // check if algorith has current task, if dont wait for one or extract it from list
            const Branch *branch = getTaskFromQueue();
            if (branch != nullptr)
            { // fetched from queue
                algorithm->setBranch(branch);
                // now safe to delete (has been copied)
                delete branch;
            }
            else
            {
                std::cout << "finish list size: " << list.size() << std::endl;
                break;
                // get task from mpi: wait
            }
        }

        // call prologue from mpi

        computeOneStep();

        // call epilogue from mpi

        // repeat
    }
}

void BranchBound::computeOneStep()
{
    BranchBoundResult *result = algorithm->computeTaskIteration();
    eBranchBoundResultType resultType = result->getResultType();
    switch (resultType)
    {
    case Solution:
    {
        BranchBoundResultSolution *resultSolution = dynamic_cast<BranchBoundResultSolution *>(result);
        int solution = resultSolution->getSolutionResult();
        if (algorithm->isBetterBound(solution))
        {
            setBound(solution);
            // send to all other worker
        }
        delete resultSolution;
        // deallocate solution
        break;
    }
    case ResultBranch:
    {
        BranchBoundResultBranch *resultBranch = dynamic_cast<BranchBoundResultBranch *>(result);
        const Branch *array = resultBranch->getArrayBranch();
        for (int i = 0; i < resultBranch->getNumberBranch(); i++)
        {
            const Branch *branch = &array[i];
            addBranchToQueue(branch);
        }
        delete resultBranch;
        // deallocate result
        break;
    }
    case Closed:
    {
        BranchBoundResultClosed *resultClose = dynamic_cast<BranchBoundResultClosed *>(result);
        delete resultClose;
        // do nothing
        break;
    }
    }
}

const Branch *BranchBound::getTaskFromQueue()
{
    const Branch *branch = nullptr;
    if (list.size() > 0)
    {
        branch = list.front();
        list.pop_front();
    }
    return branch;
}

void BranchBound::addBranchToQueue(const Branch *branch)
{
    list.push_front(branch);
}

void BranchBound::setBound(int bound)
{
    this->bound = bound;
    this->algorithm->setBound(bound);
}

std::ostream& operator<<(std::ostream& out, const BranchBound& data) {
    out << "Branch and Bound Recap:" << std::endl;
    data.algorithm->printAlgorithm(out);
    return out;
}