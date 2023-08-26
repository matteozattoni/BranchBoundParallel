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

void Knapsack::setKnapsackProblem(KnapsackProblem *knapsackProblem)
{
    this->currentSolution = new KnapsackSolution(knapsackProblem);
}

void Knapsack::setBranch(Branch *branch)
{
    if (isComputingSolution)
        throw AlgorithmAlreadyComputingBranch;
    KnapsackBranch *knapsackBranch = static_cast<KnapsackBranch *>(branch);
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
    //currentSolution->checkFeasibleSolution(); //debug
    //currentSolution->printSolution();
    const KnapsackProblem *knapsackProblem = getKnapsackProblem();
    const KnapsackProblemElement *problemElements = knapsackProblem->getKnapsackProblemElements();
    const int knapsackTotalCapacity = knapsackProblem->getTotalKnapsackCapacity();
    const KnapsackSubProblem *subProblem = computeSubSolution();

    const double upperbound = subProblem->upperbound; 
    const double lowerbound = subProblem->lowerbound;
    const double residualCapacity = subProblem->residualCapacity;
    const bool foundCritcalObject = subProblem->foundObject;
    const int idCriticalObject = subProblem->idObject;
    delete subProblem;

    const KnapsackProblemElement *criticalObject = foundCritcalObject ? &problemElements[subProblem->idObject] : nullptr;

    if (upperbound <= bound) {
        KnapsackResultClose *close = new KnapsackResultClose();
        clearSolution();
        return close;
    }

    if (foundCritcalObject == false || residualCapacity == 0) // no object can be inserted or residual capacity is 0: we close this bound with solution profit
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

    if (bound < lowerbound ) {
        KnapsackResultSolution *solution = new KnapsackResultSolution(lowerbound);
        // we don't clear the current solution (because residual is not 0 and other object can be add)
        return solution;
    }

    if (residualCapacity > 0)
    { // relaxed solution
        //cout << "upperbound is " << upperbound << " residual: " << residualCapacity << endl;

        const int solutionWeigth = currentSolution->getSolutionWeigth();
        if ((solutionWeigth + criticalObject->weight) > knapsackTotalCapacity)
        {
            currentSolution->addObjectToSolution(idCriticalObject, false, criticalObject->profit, criticalObject->weight);
            KnapsackResultClose *close = new KnapsackResultClose();
            return close;
        }
        else
        { // branch

            const int size = currentSolution->getSolutionSize();
            KnapsackBranchElement *solutionBuffer = currentSolution->getElementsSolution();

            new (&solutionBuffer[size]) KnapsackBranchElement(idCriticalObject, false);
            KnapsackBranch *newBranch = new KnapsackBranch(size + 1, size + 1, solutionBuffer);

            currentSolution->addObjectToSolution(idCriticalObject, true, criticalObject->profit, criticalObject->weight);
            KnapsackResultBranch *resultBranch = new KnapsackResultBranch(newBranch, 1);
            return resultBranch;
        }
    }

    cout << residualCapacity << endl;
    throw InfeasbileSolution;
}

Knapsack::~Knapsack()
{
}

void Knapsack::printCurrentSolution()
{
    KnapsackProblem *problem = getKnapsackProblem();
    cout << "current solutions: " << endl;
    for (int i = 0; i < problem->getProblemElementsNumber(); i++)
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

std::ostream &operator<<(std::ostream &out, Knapsack const &data)
{
    KnapsackResultSolution::printMemoryRecap(out);
    KnapsackResultBranch::printMemoryRecap(out);
    KnapsackResultClose::printMemoryRecap(out);
    KnapsackBranch::printKnapsackBranchMemory(out);
    return out;
}

std::ostream &Knapsack::printAlgorithm(std::ostream &out)
{
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

const KnapsackSubProblem* Knapsack::computeSubSolution() const {
    KnapsackProblem *knapsackProblem = dynamic_cast<KnapsackProblem *>(problem);

    if (knapsackProblem == nullptr)
        throw InvalidProblemCast;

    const KnapsackProblemElement* critEl;
    const int knapsackTotalCapacity = knapsackProblem->getTotalKnapsackCapacity();
    const int totalWeight = currentSolution->getSolutionWeigth();
    const int numberElements = problem->getProblemElementsNumber();
    double upperbound = currentSolution->getSolutionProfit();
    double residualCapacity = knapsackTotalCapacity - totalWeight;
    int idCriticalObject = -1;
    double fractProfit;
    bool foundCritcalObject = false;

    const KnapsackProblemElement *arrayProblemElements = knapsackProblem->getKnapsackProblemElements();

    for (int i = 0; i < numberElements; i++)
    {
        const KnapsackProblemElement *el = &arrayProblemElements[i];
        if (!currentSolution->hasObjectId(i))
        {
            if (residualCapacity >= el->weight)
            {
                residualCapacity -= el->weight;
                upperbound += el->profit;
            }
            else
            {
                foundCritcalObject = true;
                idCriticalObject = i;
                critEl = &arrayProblemElements[i];
                break;
            }
        }
    }
    
    double lowerResidualCapacity = residualCapacity;
    double lowerbound = upperbound;

    if (foundCritcalObject && residualCapacity > 0)
    {
        fractProfit = (residualCapacity / critEl->weight) * critEl->profit;
        upperbound += fractProfit;

        for (int i = idCriticalObject + 1; i < numberElements; i++)
        {
            const KnapsackProblemElement *el = &arrayProblemElements[i];
            if (!currentSolution->hasObjectId(i) && lowerResidualCapacity >= el->weight) {
                lowerResidualCapacity -= el->weight;
                lowerbound += el->profit;
            }
        }
    }

    return new const KnapsackSubProblem(upperbound, lowerbound, idCriticalObject, foundCritcalObject, residualCapacity);
}

void Knapsack::printMemoryInfo(std::ostream& out) {
    out << "Knapsack size classes:" << endl;
    out << "\tclass name: KnapsackProblemElement * sizeof " << sizeof(KnapsackProblemElement) << " * aligment " << alignof(KnapsackProblemElement) << endl;
    out << "\tclass name: KnapsackBranchElement * sizeof " << sizeof(KnapsackBranchElement) << " * aligment " << alignof(KnapsackBranchElement) << endl;
    out << "\tclass name: KnapsackSolution * sizeof " << sizeof(KnapsackResultSolution) << " * aligment " << alignof(KnapsackResultSolution) << endl;
}


/* SUB Problem*/

AllocatorFixedMemoryPool<KnapsackSubProblem>* KnapsackSubProblem::memoryManager = new AllocatorFixedMemoryPool<KnapsackSubProblem>();

KnapsackSubProblem::KnapsackSubProblem(double subSolution, double lowerBound, int id, bool found, double capacity): upperbound(subSolution), lowerbound(lowerBound), idObject(id), foundObject(found), residualCapacity(capacity) {}

KnapsackSubProblem::~KnapsackSubProblem() {}


void* KnapsackSubProblem::operator new(size_t size) {
    return memoryManager->allocate();
}

void KnapsackSubProblem::operator delete(void* ptr) {
    memoryManager->deallocate((KnapsackSubProblem*)ptr);
}
