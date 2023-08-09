#include "knapsackmemorymanager.h"

KnapsackMemoryManager* KnapsackMemoryManager::singleton = new KnapsackMemoryManager();

KnapsackMemoryManager::KnapsackMemoryManager(/* args */)
{
}

KnapsackMemoryManager::~KnapsackMemoryManager()
{
}

KnapsackResultSolution* KnapsackMemoryManager::allocateResultSolution() {
   return this->solutionMemoryPool.allocate();

}
KnapsackResultBranch* KnapsackMemoryManager::allocateResultBranch() {
    return this->branchMemoryPool.allocate();
}
KnapsackTask* KnapsackMemoryManager::allocateTask() {
    return this->taskMemoryPool.allocate();
}
void KnapsackMemoryManager::deallocateResultSolution(KnapsackResultSolution* ptr) {
    this->solutionMemoryPool.deallocate(ptr);
}
void KnapsackMemoryManager::deallocateResultBranch(KnapsackResultBranch* ptr) {
    this->branchMemoryPool.deallocate(ptr);
}
void KnapsackMemoryManager::deallocateTask(KnapsackTask* ptr) {
    if (ptr != nullptr)
        this->taskMemoryPool.deallocate(ptr);
}

int KnapsackMemoryManager::getNumberResultSolutionMalloc() {
    return this->solutionMemoryPool.getNumberAllocation();
}

int KnapsackMemoryManager::getNumberResultBranchMalloc() {
    return this->branchMemoryPool.getNumberAllocation();
}

int KnapsackMemoryManager::getNumberTaskMalloc() {
    return this->taskMemoryPool.getNumberAllocation();
}


KnapsackElementSolution* KnapsackMemoryManager::allocateArray(size_t size) {
    if (size <= 0)
        return nullptr;
   return this->arrayMemoryPool.allocate(size);
}

void KnapsackMemoryManager::deallocateArray(KnapsackElementSolution* ptr) {
    if (ptr != nullptr)
        this->arrayMemoryPool.deallocate(ptr);
}

int KnapsackMemoryManager::getNumberArrayCalloc() {
    return this->arrayMemoryPool.getNumberAllocation();
}