#ifndef KNAPSACKRESULT_H
#define KNAPSACKRESULT_H

#include "../branchbound/algorithm/branchboundresult.h"
#include "../branchbound/algorithm/branchboundmemory_impl.h"

class KnapsackResultSolution: public BranchBoundResultSolution {
private:
    const int profitSolution;
    /* data */
public:
    static AllocatorFixedMemoryPool<KnapsackResultSolution> *memoryManager;
    static std::ostream& printMemoryRecap(std::ostream &out) {
        out << *memoryManager;
        return out;
    };
    int getSolutionResult() override;
    void * operator new(size_t size);
    void * operator new(size_t size, void* ptr);
    void operator delete(void * p);
    KnapsackResultSolution(int profitSolution);
    ~KnapsackResultSolution();
};

// branch

class KnapsackResultBranch: public BranchBoundResultBranch
{
private:
    /* data */
public:
    static AllocatorFixedMemoryPool<KnapsackResultBranch> *memoryManager;
    static std::ostream& printMemoryRecap(std::ostream &out) {
        out << *memoryManager;
        return out;
    };
    void * operator new(size_t size);
    void * operator new(size_t size, void* ptr);
    void operator delete(void * p);
    KnapsackResultBranch(Branch* branches, int number);
    ~KnapsackResultBranch();
};

// close

class KnapsackResultClose : public BranchBoundResultClosed
{
private:
    
    /* data */
public:
    static AllocatorFixedMemoryPool<KnapsackResultClose> *memoryManager;
    static std::ostream& printMemoryRecap(std::ostream &out) {
        out << *memoryManager;
        return out;
    };
    void * operator new(size_t size);
    void * operator new(size_t size, void* ptr);
    void operator delete(void * p);
    KnapsackResultClose();
    ~KnapsackResultClose();
};

#endif