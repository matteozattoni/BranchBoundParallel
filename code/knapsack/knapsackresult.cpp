#include "knapsackresult.h"

AllocatorFixedMemoryPool<KnapsackResultSolution>* KnapsackResultSolution::memoryManager = new AllocatorFixedMemoryPool<KnapsackResultSolution>();

KnapsackResultSolution::KnapsackResultSolution(int profit): BranchBoundResultSolution() { this->profitSolution = profit;}

void* KnapsackResultSolution::operator new(size_t size) {
    void* ptr = KnapsackResultSolution::memoryManager->allocate();
    return ptr;
}

void* KnapsackResultSolution::operator new(size_t size, void* ptr) {
    return ptr;
}

void KnapsackResultSolution::operator delete(void* ptr) {
    KnapsackResultSolution::memoryManager->deallocate((KnapsackResultSolution*)ptr);
}

KnapsackResultSolution::~KnapsackResultSolution() {}

double KnapsackResultSolution::getSolutionResult() const {
    return profitSolution;
}

AllocatorFixedMemoryPool<KnapsackResultBranch>* KnapsackResultBranch::branchResultManager = new AllocatorFixedMemoryPool<KnapsackResultBranch>();

KnapsackResultBranch::KnapsackResultBranch(std::list<Branch*> listOfBranch): BranchBoundResultBranch(listOfBranch) {}

KnapsackResultBranch::~KnapsackResultBranch() {}

void* KnapsackResultBranch::operator new(size_t size) {
    void* ptr = KnapsackResultBranch::branchResultManager->allocate();
    return ptr;
}

void* KnapsackResultBranch::operator new(size_t size, void* ptr) {
    return ptr;
}

void KnapsackResultBranch::operator delete(void* ptr) {
    KnapsackResultBranch::branchResultManager->deallocate((KnapsackResultBranch*)ptr);
}


AllocatorFixedMemoryPool<KnapsackResultClose>* KnapsackResultClose::memoryManager = new AllocatorFixedMemoryPool<KnapsackResultClose>();

KnapsackResultClose::KnapsackResultClose() : BranchBoundResultClosed() {}

KnapsackResultClose::~KnapsackResultClose(){}

void* KnapsackResultClose::operator new(size_t size) {
    void* ptr = KnapsackResultClose::memoryManager->allocate();
    return ptr;
}

void* KnapsackResultClose::operator new(size_t size, void* ptr) {
    return ptr;
}

void KnapsackResultClose::operator delete(void* ptr) {
    KnapsackResultClose::memoryManager->deallocate((KnapsackResultClose*)ptr);
}
