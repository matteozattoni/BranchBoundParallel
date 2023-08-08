#ifndef KNAPSACKMEMORYMANAGER
#define KNAPSACKMEMORYMANAGER

#include "knapsackmemory.h"

class KnapsackMemoryManager
{
private:
    KnapsackStaticMemoryPool<KnapsackResultSolution> solutionMemoryPool;
    KnapsackStaticMemoryPool<KnapsackResultBranch> branchMemoryPool;
    KnapsackStaticMemoryPool<KnapsackTask> taskMemoryPool;
    KnapsackMemoryManager(/* args */);
    ~KnapsackMemoryManager();
    /* data */
public:
    static KnapsackMemoryManager* singleton;
    KnapsackResultSolution* allocateResultSolution();
    KnapsackResultBranch* allocateResultBranch();
    KnapsackTask* allocateTask();
    void deallocateResultSolution(KnapsackResultSolution* ptr);
    void deallocateResultBranch(KnapsackResultBranch* ptr);
    void deallocateTask(KnapsackTask* ptr);
};


#endif