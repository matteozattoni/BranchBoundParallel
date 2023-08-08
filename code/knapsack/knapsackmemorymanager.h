#ifndef KNAPSACKMEMORYMANAGER
#define KNAPSACKMEMORYMANAGER

#include "knapsackmemory.h"

class KnapsackMemoryManager
{
private:
    KnapsackFixedMemoryPool<KnapsackResultSolution> solutionMemoryPool;
    KnapsackFixedMemoryPool<KnapsackResultBranch> branchMemoryPool;
    KnapsackFixedMemoryPool<KnapsackTask> taskMemoryPool;
    KnapsackMemoryManager(/* args */);
    ~KnapsackMemoryManager();
    /* data */
public:
    static KnapsackMemoryManager* singleton;
    int getNumberResultSolutionMalloc();
    int getNumberResultBranchMalloc();
    int getNumberTaskMalloc();
    KnapsackResultSolution* allocateResultSolution();
    KnapsackResultBranch* allocateResultBranch();
    KnapsackTask* allocateTask();
    void deallocateResultSolution(KnapsackResultSolution* ptr);
    void deallocateResultBranch(KnapsackResultBranch* ptr);
    void deallocateTask(KnapsackTask* ptr);
};


#endif