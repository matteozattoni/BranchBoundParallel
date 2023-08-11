#ifndef KNAPSACKRESULT_H
#define KNAPSACKRESULT_H



#include "../branchbound/result.h"

class KnapsackResultSolution: public BranchBoundResultSolution {
private:
    const int profitSolution;
    /* data */
public:
    int getSolutionResult() override;
    KnapsackResultSolution(int profitSolution);
    ~KnapsackResultSolution();
};

// branch

class KnapsackResultBranch: public BranchBoundResultBranch
{
private:
    /* data */
public:
    KnapsackResultBranch(Branch* branches, int number);
    ~KnapsackResultBranch();
};

// close

class KnapsackResultClose : public BranchBoundResultClosed
{
private:
    /* data */
public:
    KnapsackResultClose();
    ~KnapsackResultClose();
};


#endif