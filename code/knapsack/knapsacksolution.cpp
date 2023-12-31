#include "knapsacksolution.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "../branchbound/branchbound.h"

using namespace std;

void KnapsackSolution::clear()
{
}

KnapsackSolution::KnapsackSolution(KnapsackProblem *p) : problem(p)
{
    solutionElements = (KnapsackBranchElement *)calloc(p->getProblemElementsNumber(), sizeof(KnapsackBranchElement));
    hasThisElementVector.reserve(problem->getProblemElementsNumber());
}

KnapsackSolution::~KnapsackSolution()
{
    free(solutionElements);
}

int KnapsackSolution::getSolutionProfit()
{
    return solutionTotalProfit;
}

int KnapsackSolution::getSolutionWeigth()
{
    return solutionTotalWeigth;
}

int KnapsackSolution::getSolutionSize()
{
    return numberSolutionElements;
}

KnapsackBranchElement *KnapsackSolution::getElementsSolution()
{
    return solutionElements;
}

void KnapsackSolution::setElementsFromBranch(KnapsackBranch *branch)
{
    solutionTotalProfit = 0;
    solutionTotalWeigth = 0;
    hasThisElementVector.assign(problem->getProblemElementsNumber(), false);
    numberSolutionElements = branch->getNumberOfElements();

    if (numberSolutionElements == 0)
        return;

    KnapsackBranchElement *elementsToCopy = (KnapsackBranchElement *)branch->getBranchElements();

    for (int i = 0; i < numberSolutionElements; i++)
    {
        int id = elementsToCopy[i].getElementId();
        bool inKnapsack = elementsToCopy[i].isInsideKnapsack();
        hasThisElementVector[id] = true;
        new(&solutionElements[i]) KnapsackBranchElement(id, inKnapsack);
    }
    
    //memcpy(solutionElements, elementsToCopy, sizeof(KnapsackBranchElement) * numberSolutionElements);

    for (int i = 0; i < numberSolutionElements; i++)
    {
        KnapsackBranchElement *el = &solutionElements[i];
        const int id = el->getElementId();
        if (el->isInsideKnapsack())
        {
            const KnapsackProblemElement *problemElem = &(this->problem->getKnapsackProblemElements()[id]);
            solutionTotalProfit += problemElem->profit;
            solutionTotalWeigth += problemElem->weight;
        }
    }

}

bool KnapsackSolution::hasObjectId(int id)
{    
    return hasThisElementVector[id];
}

void KnapsackSolution::addObjectToSolution(int id, bool in_knapsack, int profit, int weigth)
{

    if (this->problem->getProblemElementsNumber() < numberSolutionElements + 1)
    {
        printSolution();
        cout << "Overflow: trying to add id: " << id << endl;
        throw OverflowArray;
    }

    if (hasObjectId(id))
        throw ObjectAlreadyInSolution;

    if (in_knapsack)
    {
        solutionTotalProfit += profit;
        solutionTotalWeigth += weigth;
    }
    new (&solutionElements[numberSolutionElements]) KnapsackBranchElement(id, in_knapsack);
    numberSolutionElements++;
    hasThisElementVector[id] = true;
}

void KnapsackSolution::copySolutionTo(KnapsackBranchElement *buff)
{
    for (int i = 0; i < getSolutionSize(); i++)
    {
        int id = solutionElements[i].getElementId();
        bool inKnapsack = solutionElements[i].isInsideKnapsack();
        new(&buff[i]) KnapsackBranchElement(id, inKnapsack);
    }
    
    //memcpy(buff, this->solutionElements, sizeof(KnapsackBranchElement) * this->getSolutionSize());
}

void KnapsackSolution::printSolution()
{
    const KnapsackProblemElement *el = problem->getKnapsackProblemElements();
    cout << "print solution:" << endl;
    for (int i = 0; i < numberSolutionElements; i++)
    {
        int id = solutionElements[i].getElementId();
        cout << " id: " << id << "(" << el[id].profit << "-" << el[id].weight << ")"
             << " in bin: " << solutionElements[i].isInsideKnapsack() << endl;
    }
    cout << "Tot: Profit " << solutionTotalProfit << " Weight: " << solutionTotalWeigth << endl;
}

void KnapsackSolution::checkFeasibleSolution()
{

    std::set<int> idSolution;
    int knapsackWeight = problem->getTotalKnapsackCapacity();
    const KnapsackProblemElement *el = problem->getKnapsackProblemElements();
    const KnapsackBranchElement *sol = this->solutionElements;
    double totalProtit = 0;
    double totalWeigth = 0;

    for (int i = 0; i < numberSolutionElements; i++)
    {
        int id = sol[i].getElementId();
        if (idSolution.find(id) != idSolution.end())
        {
            cout << "Id already present exception" << endl;
            throw InfeasbileSolution;
        }

        idSolution.insert(id);
        if (sol[i].isInsideKnapsack())
        {
            totalProtit += el[id].profit;
            totalWeigth += el[id].weight;
        }
    }
    if (totalWeigth > knapsackWeight)
    {
        cout << "solution over weight " << totalWeigth << endl;
        throw InfeasbileSolution;
    }

    if (totalProtit != this->solutionTotalProfit || totalWeigth != this->solutionTotalWeigth)
    {
        cout << "not respect the profit/weigth solution" << endl;
        throw InfeasbileSolution;
    }
}