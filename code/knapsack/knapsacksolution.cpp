#include "knapsacksolution.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

void KnapsackSolution::clear()
{
    solutionSize = 0;
    profit = 0;
    weigth = 0;
}

KnapsackSolution::KnapsackSolution(KnapsackObject *problemElements, int problemSize)
{
    this->problemElements = problemElements;
    this->problemSize = problemSize;
    this->solutions = (KnapsackElementSolution *)calloc(problemSize, sizeof(KnapsackElementSolution));
}

KnapsackSolution::~KnapsackSolution()
{
}

int KnapsackSolution::getSolutionProfit()
{
    return profit;
}

int KnapsackSolution::getSolutionWeigth()
{
    return weigth;
}

int KnapsackSolution::getSolutionSize()
{
    return solutionSize;
}

KnapsackElementSolution *KnapsackSolution::getSolution()
{
    return solutions;
}

void KnapsackSolution::copyElements(KnapsackElementSolution *elementsToCopy, int size)
{
    solutionSize = size;
    if (solutionSize == 0)
    {
        profit = 0;
        weigth = 0;
        return;
    }

    
    memcpy(solutions, elementsToCopy, sizeof(KnapsackElementSolution)*solutionSize);
    for (size_t i = 0; i < solutionSize; i++)
    {
        if (solutions[i].in_knapsack)
        {
            int solutionElementId = solutions[i].id;
            KnapsackObject ob = problemElements[solutionElementId];
            profit += ob.profit;
            weigth += ob.weight;
        }
    }
}

bool KnapsackSolution::hasObjectId(int id)
{
    bool hasThisId = false;
    for (size_t i = 0; i < solutionSize; i++)
    {
        hasThisId = hasThisId || solutions[i].id == id;
    }
    return hasThisId;
}

void KnapsackSolution::addObjectToSolution(int id, bool in_knapsack, int profit, int weigth)
{

    if (this->problemSize < solutionSize + 1)
    {
        cout << "overflow: " << this->problemSize << " " << solutionSize << "trying to add this ob " << id << endl;
        printSolution();
        throw;
    }

    if (hasObjectId(id))
    {
        cout << "Error: already in the solution id: " << id << endl;
        throw;
    }

    if (in_knapsack)
    {
        this->profit += profit;
        this->weigth += weigth;
    }
    solutions[solutionSize].id = id;
    solutions[solutionSize].in_knapsack = in_knapsack;
    solutionSize++;
}

void KnapsackSolution::printSolution()
{
    cout << "print solution:" << endl;
    for (size_t i = 0; i < solutionSize; i++)
    {
        cout << " id: " << solutions[i].id << " in bin: " << solutions[i].in_knapsack << endl;
    }
}