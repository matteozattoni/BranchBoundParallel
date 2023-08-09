#ifndef KNAPSACKMEMORYMANAGER
#define KNAPSACKMEMORYMANAGER

#include "knapsackmemory.h"
#include <ostream>



class KnapsackMemoryRecap {
public:
    size_t totalSizeRequestedArray;
    size_t totalSizeAllocatedArray;
    int totalCallSolution;
    int totalAllocationSolution;
    int totalCallBranch;
    int totalAllocationBranch;
    int totalCallTask;
    int totalAllocationTask;
    int totalCallArrayElem;
    int totalAllocationArray;
    KnapsackMemoryRecap(size_t,size_t,int,int,int,int,int,int,int,int);

    friend std::ostream& operator <<(std::ostream &out, KnapsackMemoryRecap const& data);
    size_t totalSizeBranchRequest() const;
    size_t totalSizeAllocatedBranch() const;
    size_t totalSizeSolutionRequest() const;
    size_t totalSizeAllocatedSolution() const;
    size_t totalSizeTaskRequest() const;
    size_t totalSizeAllocatedTask() const;

};


class KnapsackMemoryManager
{
private:
    int numberCallAllocationSolution = 0;
    int numberCallAllocationBranch = 0;
    int numberCallAllocationTask = 0;
    int numberCallAllocationArray = 0;
    size_t sizeRequestedForArray = 0;
    KnapsackFixedMemoryPool<KnapsackResultSolution> solutionMemoryPool;
    KnapsackFixedMemoryPool<KnapsackResultBranch> branchMemoryPool;
    KnapsackFixedMemoryPool<KnapsackTask> taskMemoryPool;
    KnapsackArrayMemoryPool<KnapsackElementSolution> arrayMemoryPool;
    KnapsackMemoryManager(/* args */);
    ~KnapsackMemoryManager();
    /* data */
public:
    static KnapsackMemoryManager* singleton;
    KnapsackMemoryRecap* getMemoryInfo();
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