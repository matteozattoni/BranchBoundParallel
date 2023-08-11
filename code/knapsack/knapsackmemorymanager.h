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
    int totalCallClose;
    int totalAllocationClose;
    int totalCallBranchResult;
    int totalAllocationBranchResult;
    int totalCallBranch;
    int totalAllocationBranch;
    int totalCallArrayElem;
    int totalAllocationArray;
    KnapsackMemoryRecap(size_t,size_t,int,int,int,int,int,int,int,int,int,int);

    friend std::ostream& operator <<(std::ostream &out, KnapsackMemoryRecap const& data);
    size_t totalSizeBranchResultRequest() const;
    size_t totalSizeAllocatedBranchResult() const;
    size_t totalSizeCloseRequest() const;
    size_t totalSizeAllocatedClose() const;
    size_t totalSizeSolutionRequest() const;
    size_t totalSizeAllocatedSolution() const;
    size_t totalSizeBranch() const;
    size_t totalSizeAllocatedBranch() const;

};


class KnapsackMemoryManager
{
private:
    int numberCallAllocationSolution = 0;
    int numberCallAllocationClose = 0;
    int numberCallAllocationResultBranch = 0;
    int numberCallAllocationBranch = 0;
    int numberCallAllocationArray = 0;
    size_t sizeRequestedForArray = 0;
    KnapsackFixedMemoryPool<KnapsackResultSolution> solutionMemoryPool;
    KnapsackFixedMemoryPool<KnapsackResultBranch> branchResultMemoryPool;
    KnapsackFixedMemoryPool<KnapsackResultClose> closeResultMemoryPool;
    KnapsackFixedMemoryPool<KnapsackBranch> branchMemoryPool;
    //KnapsackArrayMemoryPool<KnapsackElementSolution> arrayMemoryPool = KnapsackArrayMemoryPool<KnapsackElementSolution>(0.2,50);
    KnapsackArrayMemoryPool<KnapsackBranchElement> arrayMemoryPool = KnapsackArrayMemoryPool<KnapsackBranchElement>();
    KnapsackMemoryManager(/* args */);
    ~KnapsackMemoryManager();
public:
    static KnapsackMemoryManager* singleton;
    KnapsackMemoryRecap* getMemoryInfo();
    KnapsackResultSolution* allocateResultSolution();
    KnapsackResultClose* allocateResultClose();
    KnapsackResultBranch* allocateResultBranch();
    KnapsackBranch* allocateBranch();
    KnapsackBranchElement* allocateArray(size_t size);
    void deallocateResultSolution(KnapsackResultSolution* ptr);
    void deallocateResultClose(KnapsackResultClose* ptr);
    void deallocateResultBranch(KnapsackResultBranch* ptr);
    void deallocateTask(KnapsackBranch* ptr);
    void deallocateArray(KnapsackBranchElement* ptr);
};

#endif