#include "knapsackresult.h"

KnapsackResultSolution::KnapsackResultSolution(int profit): BranchBoundResultSolution(), profitSolution(profit) {}

KnapsackResultSolution::~KnapsackResultSolution() {}

int KnapsackResultSolution::getSolutionResult() {
    return profitSolution;
}

KnapsackResultBranch::KnapsackResultBranch(Branch* branches, int number): BranchBoundResultBranch(branches, number) {}

KnapsackResultBranch::~KnapsackResultBranch() {}


KnapsackResultClose::KnapsackResultClose() : BranchBoundResultClosed() {}

KnapsackResultClose::~KnapsackResultClose(){}