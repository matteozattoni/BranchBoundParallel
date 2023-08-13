#ifndef KNAPSACKCLASS
#define KNAPSACKCLASS

#include <ostream>

#include "../branchbound/branchboundalgorithm.h"
#include "../branchbound/branchboundproblem.h"
#include "../branchbound/result.h"
#include "../branchbound/branchboundexception.h"
#include "knapsacksolution.h"
#include "knapsackproblem.h"
#include "knapsackresult.h"


using namespace std;

class KnapsackSubProblem
{
private:
    static AllocatorFixedMemoryPool<KnapsackSubProblem>* memoryManager;
    /* data */
public:
    const double upperbound;
    const int idObject;
    const bool foundObject;
    const double residualCapacity;
    KnapsackSubProblem(double bound, int id, bool isFoundObject, double capacity);
    ~KnapsackSubProblem();
    void * operator new(size_t size);
    void operator delete(void * p);
};


class Knapsack: public BranchBoundAlgorithm {

private:
    int bound = -1;
    bool isComputingSolution = false;
    KnapsackSolution* currentSolution;
    void clearSolution();

    void setKnapsackProblem(KnapsackProblem*);
    KnapsackProblem* getKnapsackProblem();
    const KnapsackSubProblem* computeSubSolution(const KnapsackBranchElement* elem, int numb) const;
    const KnapsackSubProblem* computeSubSolution() const;
public:
    Knapsack(/* manager?*/);
    ~Knapsack();

    bool isBetterBound(int bound) override;
    void setBound(int bound) override;
    void setProblem(BranchBoundProblem*) override;
    void setProblemWithRootBranch(BranchBoundProblem*) override;
    void setBranch(const Branch*) override;
    BranchBoundResult* computeTaskIteration() override;
    bool hasCurrentBranch() override;
    std::ostream& printAlgorithm(std::ostream& out) override;

    void printCurrentSolution();
    friend std::ostream& operator <<(std::ostream &out, Knapsack const& data);

};


#endif