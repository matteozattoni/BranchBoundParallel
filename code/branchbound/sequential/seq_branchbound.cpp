#include "seq_branchbound.h"
#include <iostream>

SequentialBranchBound::SequentialBranchBound(BranchBoundAlgorithm *algorithm)
{
    this->algorithm = algorithm;
}

SequentialBranchBound::~SequentialBranchBound()
{
}

void SequentialBranchBound::start(BranchBoundProblem *problem, Branch *rootBranch) {

    algorithm->setProblem(problem);
    algorithm->setBranch(rootBranch);
    delete rootBranch;

    while (algorithm->hasCurrentBranch() || !list.empty())
    {
        if (!algorithm->hasCurrentBranch()) {
            Branch *branch = list.front();
            list.pop_front();
            algorithm->setBranch(branch);
            delete branch;
        }

        BranchBoundResult *result = algorithm->computeTaskIteration();
        newBranchBoundResult(result);
    }
    
}

void SequentialBranchBound::newBranchBoundResult(BranchBoundResult* result) {
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
        }
        delete resultSolution;
        break;
    }
    case Branches:
    {
        BranchBoundResultBranch *resultBranch = dynamic_cast<BranchBoundResultBranch *>(result);
        for(Branch* b : resultBranch->getListBranch()) {
            list.push_front(b);
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

void SequentialBranchBound::setBound(int bound)
{    
    this->bound = bound;
    this->algorithm->setBound(bound);
}

int SequentialBranchBound::getBound() {
    return bound;
}