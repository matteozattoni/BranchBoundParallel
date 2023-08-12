#ifndef KNAPSACKRESULT_H
#define KNAPSACKRESULT_H

#include "../branchbound/result.h"
#include "../branchbound/branchboundmemory_impl.h"

class KnapsackResultSolution: public BranchBoundResultSolution {
private:
    const int profitSolution;
    static AllocatorFixedMemoryPool<KnapsackResultSolution> *memoryManager;
    /* data */
public:
    static std::ostream& printMemoryRecap(std::ostream &out) {
        out << *memoryManager;
        return out;
    };
    int getSolutionResult() override;
    void * operator new(size_t size);
    void operator delete(void * p);
    KnapsackResultSolution(int profitSolution);
    ~KnapsackResultSolution();
};

// branch

class KnapsackResultBranch: public BranchBoundResultBranch
{
private:
    static AllocatorFixedMemoryPool<KnapsackResultBranch> *memoryManager;
    /* data */
public:
    static std::ostream& printMemoryRecap(std::ostream &out) {
        out << *memoryManager;
        return out;
    };
    void * operator new(size_t size);
    void operator delete(void * p);
    KnapsackResultBranch(Branch* branches, int number);
    ~KnapsackResultBranch();
};

// close

class KnapsackResultClose : public BranchBoundResultClosed
{
private:
    static AllocatorFixedMemoryPool<KnapsackResultClose> *memoryManager;
    /* data */
public:
    static std::ostream& printMemoryRecap(std::ostream &out) {
        out << *memoryManager;
        return out;
    };
    void * operator new(size_t size);
    void operator delete(void * p);
    KnapsackResultClose();
    ~KnapsackResultClose();
};

#endif