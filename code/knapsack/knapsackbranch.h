#ifndef KNAPSACKBRANCH_H
#define KNAPSACKBRANCH_H

#include "../branchbound/branchboundbranch.h"
#include "../branchbound/branchboundexception.h"
#include "../branchbound/branchboundmemory_impl.h"

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
    static AllocatorFixedMemoryPool<KnapsackBranch>* branchMemoryManager;
    static AllocatorArrayMemoryPool<KnapsackBranchElement>* elementsMemoryManager;
    const int bufferDimension;
    /* data */
public:
    static std::ostream& printKnapsackBranchMemory(std::ostream& out);
    const KnapsackBranchElement *getKnapsackBranchElement() const;
    void * operator new(size_t size);
    void operator delete(void * p);
    KnapsackBranch(double upperbound, int dimensionBuffer, int numberElemSolution, BranchElement * buffSolution);
    ~KnapsackBranch();
};

template class AllocatorFixedMemoryPool<KnapsackBranch>;
template class AllocatorArrayMemoryPool<KnapsackBranchElement>;

#endif