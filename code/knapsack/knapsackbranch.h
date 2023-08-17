#ifndef KNAPSACKBRANCH_H
#define KNAPSACKBRANCH_H

#include "../branchbound/algorithm/branchboundbranch.h"
#include "../branchbound/algorithm/branchboundexception.h"
#include "../branchbound/algorithm/branchboundmemory_impl.h"

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
    const int bufferDimension;
    /* data */
public:
    static AllocatorFixedMemoryPool<KnapsackBranch>* branchMemoryManager;
    static AllocatorArrayMemoryPool<KnapsackBranchElement>* elementsMemoryManager;
    static std::ostream& printKnapsackBranchMemory(std::ostream& out);
    static const KnapsackBranch &rootBranch;
    const KnapsackBranchElement *getKnapsackBranchElement() const;
    void * operator new(size_t size);
    void * operator new(size_t size, void* ptr);
    void operator delete(void * p);
    KnapsackBranch(int numElements, BranchElement* buff);
    KnapsackBranch(int dimensionBuffer, int numberElemSolution, BranchElement *buffSolution);
    ~KnapsackBranch();
};


#endif