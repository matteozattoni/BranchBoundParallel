#include "knapsackresult.h"

KnapsackResultSolution::KnapsackResultSolution(int profitSolution)
{
    this->profitSolution = profitSolution;
}

KnapsackResultSolution::~KnapsackResultSolution()
{

}

int KnapsackResultSolution::getSolutionResult() {
    return profitSolution;
}

KnapsackResultBranch::KnapsackResultBranch(KnapsackTask* tasks, int num)
{
    numberOfTask = num;
    this->tasks = tasks;
}

KnapsackResultBranch::~KnapsackResultBranch()
{
}

int KnapsackResultBranch::getNumberBranch() {
    return numberOfTask;
}

void* KnapsackResultBranch::getArrayBranch() {
    return (void*) tasks;
}