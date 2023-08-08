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
    this->taskMemoryPool.deallocate(ptr);
}