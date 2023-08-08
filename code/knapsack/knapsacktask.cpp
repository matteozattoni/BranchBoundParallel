#include "knapsacktask.h"

#include <iostream>

using namespace std;

KnapsackTask::KnapsackTask(KnapsackElementSolution *ob, int size)
{
    this->objects = ob;
    this->size = size;
}

void KnapsackTask::copyFromSolution(KnapsackSolution *solution, int newElemId, bool inKnapsack)
{
    
    int size = solution->getSolutionSize();
    if (size > 0)
    {
        KnapsackElementSolution *solutionBuffer = solution->getSolution();
        KnapsackElementSolution *buffToCopy = (KnapsackElementSolution *)calloc(size + 1, sizeof(KnapsackElementSolution));
        memcpy(buffToCopy, solutionBuffer, sizeof(KnapsackElementSolution) * size);
        cout << newElemId << " " << inKnapsack << endl;
        buffToCopy[size].id = newElemId;
        buffToCopy[size].in_knapsack = inKnapsack;
        this->objects = buffToCopy;
        this->size = size+1;
    }
    else
    {
        this->objects = (KnapsackElementSolution *)malloc(sizeof(KnapsackElementSolution));
        this->size = 1;
    }
}

void KnapsackTask::setBuffer(KnapsackElementSolution *buff, int size)
{
    this->objects = buff;
    this->size = 0;
}

KnapsackTask::KnapsackTask(KnapsackSolution *solution, int newElemId, bool inKnapsack)
{
    int size = solution->getSolutionSize();
    if (size > 0)
    {
        KnapsackElementSolution *solutionBuffer = solution->getSolution();
        KnapsackElementSolution *buffToCopy = (KnapsackElementSolution *)calloc(size + 1, sizeof(KnapsackElementSolution));
        memcpy(buffToCopy, solutionBuffer, sizeof(KnapsackElementSolution) * size);
        buffToCopy[size].id = newElemId;
        buffToCopy[size].in_knapsack = inKnapsack;
        this->objects = buffToCopy;
        this->size = size+1;
    }
    else
    {
        this->objects = (KnapsackElementSolution*)calloc(1, sizeof(KnapsackElementSolution));
        this->objects[0].id = newElemId;
        this->objects[0].in_knapsack = inKnapsack;
        this->size = 1;
    }
}

KnapsackTask::~KnapsackTask()
{
}
