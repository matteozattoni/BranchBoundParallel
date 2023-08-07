#ifndef KNAPSACKRESULT
#define KNAPSACKRESULT



#include "../branchbound/result.h"
#include "knapsacksolution.h"
#include "knapsacktask.h"

class KnapsackResultSolution: public virtual BranchBoundResultSolution {
private:
    int profitSolution;
    /* data */
public:
    int getSolutionResult() override;
    KnapsackResultSolution(int profitSolution);
    ~KnapsackResultSolution();
};

// branch

class KnapsackResultBranch: public virtual BranchBoundResultBranch
{
private:
    int numberOfTask;
    KnapsackTask* tasks;
    /* data */
public:
    int getNumberBranch() override;
    void* getArrayBranch() override;
    KnapsackResultBranch(KnapsackTask* tasks, int num);
    ~KnapsackResultBranch();
};


#endif