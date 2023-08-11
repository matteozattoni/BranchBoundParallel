#ifndef KNAPSACKBRANCH_H
#define KNAPSACKBRANCH_H

#include "../branchbound/branchboundbranch.h"
#include "../branchbound/branchboundexception.h"

class KnapsackBranchElement : public BranchElement
{
private:
    const bool insideKnapsack;
    /* data */
public:
    bool isInsideKnapsack() const { return insideKnapsack; }
    KnapsackBranchElement(int, bool);
    ~KnapsackBranchElement();
};

class KnapsackBranch : public Branch
{
private:
    /* data */
public:
    const KnapsackBranchElement *getKnapsackBranchElement() const;
    KnapsackBranch(double, int, BranchElement *);
    ~KnapsackBranch();
};

#endif