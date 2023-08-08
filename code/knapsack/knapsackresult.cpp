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

void KnapsackResultBranch::setTasks(KnapsackTask* tasks, int num) {
    this->tasks = tasks;
    this->numberOfTask = num;
}

int KnapsackResultBranch::getNumberBranch() {
    return numberOfTask;
}

void* KnapsackResultBranch::getArrayBranch() {
    return (void*) tasks;
}