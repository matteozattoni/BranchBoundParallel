#include "knapsack.h"
#include <string.h>
#include <bits/stdc++.h>

using namespace std;

bool Knapsack::hasCurrentTask()
{
    return this->isComputingSolution;
}

void Knapsack::setCurrentTask(KnapsackTask *task)
{
    if (isComputingSolution)
        throw;
    isComputingSolution = true;
    currentSolution->solutionFromTask(task);
}

void Knapsack::setBound(int bound)
{
    if (this->bound < bound)
        this->bound = bound;
}

void Knapsack::clearSolution()
{
    isComputingSolution = false;
    currentSolution->clear();
}

BranchBoundResult *Knapsack::computeTaskIteration()
{
    if (!isComputingSolution)
        throw;
    int idCriticalObject;
    KnapsackObject *criticalObject;
    int solutionCapacity = currentSolution->getSolutionWeigth();
    double upperbound = currentSolution->getSolutionProfit();
    double fract_profit;
    
    double residualCapacity = knapsackCapacity - solutionCapacity;
    bool foundCritcalObject = false;
    int i = 0;
    while (i < problemDimension) // Relaxing: upperbound is the real number global solution
    {
        if (!(currentSolution->hasObjectId(i)))
        {
            KnapsackObject object = problemElements[i];
            if (residualCapacity >= object.weight)
            {
                residualCapacity -= object.weight;
                upperbound += object.profit;
            }
            else
            {
                foundCritcalObject = true;
                idCriticalObject = i;
                criticalObject = &problemElements[idCriticalObject];
                break;
            }
        }
        i++;
    }

    if (residualCapacity == 0)
    {
        clearSolution();
        KnapsackResultSolution* my = solutionMemoryPool.allocate();
        return new(my) KnapsackResultSolution(upperbound);
    }

    if (foundCritcalObject == false)
    { // no object can be inserted: we close this bound with solution profit
        int profit = currentSolution->getSolutionProfit();
        clearSolution();
        KnapsackResultSolution* my = solutionMemoryPool.allocate();
        return new(my) KnapsackResultSolution(profit);
    }

    if (upperbound <= bound)
    {
        clearSolution();
        KnapsackResultSolution* my = solutionMemoryPool.allocate();
        return new(my) KnapsackResultSolution(upperbound);
    }

    if (residualCapacity > 0)
    { // relaxed solution
        fract_profit = (residualCapacity / criticalObject->weight) * criticalObject->profit;
        upperbound += fract_profit;
        int solutionWeigth = currentSolution->getSolutionWeigth();
        if ((solutionWeigth + criticalObject->weight) > knapsackCapacity)
        {
            currentSolution->addObjectToSolution(i, false, criticalObject->profit, criticalObject->weight);
            KnapsackResultBranch* branch = branchMemoryPool.allocate();
            return new(branch) KnapsackResultBranch(nullptr, 0);
        }
        else
        { // actual branch
            int currentSolutionSize = currentSolution->getSolutionSize();
            KnapsackElementSolution *currentSolutionBuffer = currentSolution->getSolution();
            KnapsackElementSolution *buffTask = KnapsackTask::getTaskBuffer(currentSolutionSize + 1);
            memcpy(buffTask, currentSolutionBuffer, sizeof(KnapsackElementSolution) * currentSolutionSize);
            buffTask[currentSolutionSize].id = i;
            buffTask[currentSolutionSize].in_knapsack = false;
            KnapsackTask *task = new KnapsackTask(buffTask, currentSolutionSize + 1);
            currentSolution->addObjectToSolution(i, true, criticalObject->profit, criticalObject->weight);
            KnapsackResultBranch* branch = branchMemoryPool.allocate();
            return new(branch) KnapsackResultBranch(task, 1);
        }
    }
    throw;
}

Knapsack::Knapsack(KnapsackObject *problemElements, int problemDimension, int knapsackCapacity)
{
    this->problemElements = problemElements;
    this->problemDimension = problemDimension;
    this->knapsackCapacity = knapsackCapacity;
    this->currentSolution = new KnapsackSolution(problemElements, problemDimension);
}

Knapsack::~Knapsack()
{
}

void Knapsack::printCurrentSolution()
{
    cout << "current solutions: " << endl;
    for (size_t i = 0; i < problemDimension; i++)
    {
        KnapsackObject *ob = &problemElements[i];
        int obId = i;
        if (currentSolution->hasObjectId(obId))
        {
            cout << "Id: " << obId << " profit: " << ob->profit << " weigth: " << ob->weight << endl;
        }
    }

    cout << "Solution -- profit: " << currentSolution->getSolutionProfit() << " weigth: " << currentSolution->getSolutionWeigth() << " size: " << currentSolution->getSolutionSize() << endl;
}