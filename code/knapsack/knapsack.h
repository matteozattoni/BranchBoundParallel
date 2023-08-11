#ifndef KNAPSACKCLASS
#define KNAPSACKCLASS

#include "../branchbound/branchboundalgorithm.h"
#include "../branchbound/branchboundproblem.h"
#include "../branchbound/result.h"
#include "../branchbound/branchboundexception.h"
#include "knapsacksolution.h"
#include "knapsackproblem.h"
#include "knapsackresult.h"
#include "knapsackmemorymanager.h"


using namespace std;

class Knapsack: public BranchBoundAlgorithm {

private:
    int bound = -1;
    bool isComputingSolution = false;
    KnapsackSolution* currentSolution;
    void clearSolution();
    KnapsackMemoryManager* manager;

    void setKnapsackProblem(KnapsackProblem*);
    KnapsackProblem* getKnapsackProblem();
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

    void printCurrentSolution();


};


#endif