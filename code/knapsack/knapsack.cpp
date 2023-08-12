#include "knapsack.h"
#include <string.h>
#include <iostream>

using namespace std;

Knapsack::Knapsack() {}

KnapsackProblem *Knapsack::getKnapsackProblem()
{
    KnapsackProblem *knapsackProblem = static_cast<KnapsackProblem *>(problem);
    if (knapsackProblem == nullptr)
        throw InvalidProblemCast;
    return knapsackProblem;
}

bool Knapsack::isBetterBound(int bound)
{
    return this->bound < bound;
}

void Knapsack::setProblem(BranchBoundProblem *problem)
{
    this->problem = problem;
    KnapsackProblem *knapsackProblem = static_cast<KnapsackProblem *>(problem);
    if (knapsackProblem == nullptr)
        throw InvalidProblemCast;
    setKnapsackProblem(knapsackProblem);
}

void Knapsack::setProblemWithRootBranch(BranchBoundProblem *problem)
{
    setProblem(problem);
    const KnapsackBranch *knapsackBranch = new KnapsackBranch(0.0, 0, 0, nullptr);
    setBranch(knapsackBranch);
}

void Knapsack::setKnapsackProblem(KnapsackProblem *knapsackProblem)
{
    this->currentSolution = new KnapsackSolution(knapsackProblem);
}

void Knapsack::setBranch(const Branch *branch)
{
    if (isComputingSolution)
        throw AlgorithmAlreadyComputingBranch;
    const KnapsackBranch *knapsackBranch = static_cast<const KnapsackBranch *>(branch);
    if (knapsackBranch == nullptr)
        throw InvalidBranchCast;
    isComputingSolution = true;
    currentSolution->setElementsFromBranch(knapsackBranch);
}

bool Knapsack::hasCurrentBranch()
{
    return this->isComputingSolution;
}

void Knapsack::setBound(int bound)
{
    if (this->bound < bound)
        this->bound = bound;
}

void Knapsack::clearSolution()
{
    isComputingSolution = false;
    // currentSolution->clear();
}

BranchBoundResult *Knapsack::computeTaskIteration()
{
    if (!isComputingSolution)
        throw AlgorithmIsNotComputingBranch;
    currentSolution->checkFeasibleSolution();
    KnapsackProblem *knapsackProblem = getKnapsackProblem();
    const KnapsackProblemElement *problemElements = knapsackProblem->getKnapsackProblemElements();
    const int problemDimension = knapsackProblem->getProblemElementsNumber();
    const int knapsackTotalCapacity = knapsackProblem->getTotalKnapsackCapacity();
    const int solutionCapacity = currentSolution->getSolutionWeigth();

    int idCriticalObject;
    const KnapsackProblemElement *criticalObject;

    double upperbound = currentSolution->getSolutionProfit();
    double fract_profit;

    double residualCapacity = knapsackTotalCapacity - solutionCapacity;
    bool foundCritcalObject = false;
    int i = 0;
    while (i < problemDimension) // Relaxing: upperbound is the real number global solution
    {
        if (!(currentSolution->hasObjectId(i)))
        {
            const KnapsackProblemElement object = problemElements[i];
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
        if (upperbound > bound)
        {
            KnapsackResultSolution *solution = new KnapsackResultSolution(upperbound);
            clearSolution();
            return solution;
        }
        else
        {
            KnapsackResultClose *close = new KnapsackResultClose();
            clearSolution();
            return close;
        }
    }

    if (foundCritcalObject == false)
    { // no object can be inserted: we close this bound with solution profit
        if (upperbound > bound)
        {
            KnapsackResultSolution *solution = new KnapsackResultSolution(upperbound);
            clearSolution();
            return solution;
        }
        else
        {
            KnapsackResultClose *close = new KnapsackResultClose();
            clearSolution();
            return close;
        }
    }


    if (residualCapacity > 0)
    { // relaxed solution
        fract_profit = (residualCapacity / criticalObject->weight) * criticalObject->profit;
        upperbound += fract_profit;
        int solutionWeigth = currentSolution->getSolutionWeigth();
        if ((solutionWeigth + criticalObject->weight) > knapsackTotalCapacity)
        {
            currentSolution->addObjectToSolution(idCriticalObject, false, criticalObject->profit, criticalObject->weight);
            KnapsackResultClose *close = new KnapsackResultClose();
            return close;
        }
        else
        { // branch
            
            int size = currentSolution->getSolutionSize();
            double p = currentSolution->getSolutionProfit();
            
            KnapsackBranchElement* solutionBuffer = currentSolution->getElementsSolution();
            new(&solutionBuffer[size]) KnapsackBranchElement(idCriticalObject, false);
            KnapsackBranch *newBranch = new KnapsackBranch(p, size+1, size+1, solutionBuffer);// seg fault
            
            currentSolution->addObjectToSolution(idCriticalObject, true, criticalObject->profit, criticalObject->weight);
            KnapsackResultBranch *resultBranch = new KnapsackResultBranch(newBranch, 1);
            return resultBranch;
        }
    }
    throw;
}

Knapsack::~Knapsack()
{
}

void Knapsack::printCurrentSolution()
{
    KnapsackProblem *problem = getKnapsackProblem();
    cout << "current solutions: " << endl;
    for (size_t i = 0; i < problem->getProblemElementsNumber(); i++)
    {
        const KnapsackProblemElement *ob = &problem->getKnapsackProblemElements()[i];
        int obId = i;
        if (currentSolution->hasObjectId(obId))
        {
            cout << "Id: " << obId << " profit: " << ob->profit << " weigth: " << ob->weight << endl;
        }
    }

    cout << "Solution -- profit: " << currentSolution->getSolutionProfit() << " weigth: " << currentSolution->getSolutionWeigth() << " size: " << currentSolution->getSolutionSize() << endl;
}

std::ostream& operator <<(std::ostream &out, Knapsack const& data) {
    KnapsackResultSolution::printMemoryRecap(out);
    KnapsackResultBranch::printMemoryRecap(out);
    KnapsackResultClose::printMemoryRecap(out);
    KnapsackBranch::printKnapsackBranchMemory(out);
    return out;
}

std::ostream& Knapsack::printAlgorithm(std::ostream& out) {
    out << "\t\t*Knapsack Algorithm*" << std::endl;
    out << "KnapsackResultSolution" << std::endl;
    KnapsackResultSolution::printMemoryRecap(out);
    out << "\nKnapsackResultBranch" << std::endl;
    KnapsackResultBranch::printMemoryRecap(out);
    out << "\nKnapsackResultClose" << std::endl;
    KnapsackResultClose::printMemoryRecap(out);
    out << "\nKnapsackBranch" << std::endl;
    KnapsackBranch::printKnapsackBranchMemory(out);
    return out;
}