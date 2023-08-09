#ifndef KNAPSACKMEMORYMANAGER
#define KNAPSACKMEMORYMANAGER

#include "knapsackmemory.h"

class KnapsackMemoryManager
{
private:
    KnapsackFixedMemoryPool<KnapsackResultSolution> solutionMemoryPool;
    KnapsackFixedMemoryPool<KnapsackResultBranch> branchMemoryPool;
    KnapsackFixedMemoryPool<KnapsackTask> taskMemoryPool;
    KnapsackArrayMemoryPool<KnapsackElementSolution> arrayMemoryPool;
    KnapsackMemoryManager(/* args */);
    ~KnapsackMemoryManager();
    /* data */
public:
    static KnapsackMemoryManager* singleton;
    int getNumberResultSolutionMalloc();
    int getNumberResultBranchMalloc();
    int getNumberTaskMalloc();
    int getNumberArrayCalloc();
    KnapsackResultSolution* allocateResultSolution();
    KnapsackResultBranch* allocateResultBranch();
    KnapsackTask* allocateTask();
    KnapsackElementSolution* allocateArray(size_t size);
    void deallocateResultSolution(KnapsackResultSolution* ptr);
    void deallocateResultBranch(KnapsackResultBranch* ptr);
    void deallocateTask(KnapsackTask* ptr);
    void deallocateArray(KnapsackElementSolution* ptr);
};


#endif