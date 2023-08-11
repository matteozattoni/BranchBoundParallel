#include "knapsackmemorymanager.h"

KnapsackMemoryManager* KnapsackMemoryManager::singleton = new KnapsackMemoryManager();

KnapsackMemoryManager::KnapsackMemoryManager(/* args */)
{
}

KnapsackMemoryManager::~KnapsackMemoryManager()
{
}

KnapsackResultSolution* KnapsackMemoryManager::allocateResultSolution() {
    numberCallAllocationSolution++;
   return this->solutionMemoryPool.allocate();

}

KnapsackResultClose* KnapsackMemoryManager::allocateResultClose() {
    numberCallAllocationClose++;
    return this->closeResultMemoryPool.allocate();
}

KnapsackResultBranch* KnapsackMemoryManager::allocateResultBranch() {
    numberCallAllocationResultBranch++;
    return this->branchResultMemoryPool.allocate();
}
KnapsackBranch* KnapsackMemoryManager::allocateBranch() {
    numberCallAllocationBranch++;
    return this->branchMemoryPool.allocate();
}
void KnapsackMemoryManager::deallocateResultSolution(KnapsackResultSolution* ptr) {
    this->solutionMemoryPool.deallocate(ptr);
}
void KnapsackMemoryManager::deallocateResultClose(KnapsackResultClose* ptr) {
    this->closeResultMemoryPool.deallocate(ptr);
}
void KnapsackMemoryManager::deallocateResultBranch(KnapsackResultBranch* ptr) {
    this->branchResultMemoryPool.deallocate(ptr);
}
void KnapsackMemoryManager::deallocateTask(KnapsackBranch* ptr) {
    if (ptr != nullptr)
        this->branchMemoryPool.deallocate(ptr);
}

KnapsackBranchElement* KnapsackMemoryManager::allocateArray(size_t size) {
    numberCallAllocationArray++;
    sizeRequestedForArray+=size;
    if (size <= 0)
        return nullptr;
   return this->arrayMemoryPool.allocate(size);
}

void KnapsackMemoryManager::deallocateArray(KnapsackBranchElement* ptr) {
    if (ptr != nullptr)
        this->arrayMemoryPool.deallocate(ptr);
}

KnapsackMemoryRecap* KnapsackMemoryManager::getMemoryInfo() {
    size_t totArr = this->arrayMemoryPool.getTotalSizeAllocated();
    int allSol = this->solutionMemoryPool.getNumberOfMalloc();
    int callSol = numberCallAllocationSolution;
    int allBra = this->branchResultMemoryPool.getNumberOfMalloc();
    int callBra = numberCallAllocationResultBranch;
    int allTask = this->branchMemoryPool.getNumberOfMalloc();
    int callTask = numberCallAllocationBranch;
    int allArr = this->arrayMemoryPool.getNumberAllocation();
    int callArr = numberCallAllocationArray;
    int callClose = numberCallAllocationClose;
    int allClose = this->closeResultMemoryPool.getNumberOfMalloc();
    KnapsackMemoryRecap* info = new KnapsackMemoryRecap(totArr, sizeRequestedForArray, allSol, callSol, allBra, callBra, allTask, callTask, allArr, callArr, allClose, callClose);
    return info;
}

KnapsackMemoryRecap::KnapsackMemoryRecap(size_t totArr, size_t reqArr, int allSol, int callSol, int allBra, int callBra, int allTask, int callTask, int allArr, int callArr, int allClose, int callClose) {
    this->totalSizeAllocatedArray = totArr;
    this->totalSizeRequestedArray = reqArr;
    this->totalAllocationSolution = allSol;
    this->totalCallSolution = callSol;
    this->totalCallBranchResult = callBra;
    this->totalAllocationBranchResult = allBra;
    this->totalCallBranch = callTask;
    this->totalAllocationBranch = allTask;
    this->totalCallArrayElem = callArr;
    this->totalAllocationArray = allArr;
    this->totalCallClose = callClose;
    this->totalAllocationClose = allClose;
}


std::ostream& operator <<(std::ostream &out, KnapsackMemoryRecap const& data) {
    size_t solutionGain = data.totalSizeSolutionRequest() - data.totalSizeAllocatedSolution();
    size_t closeGain = data.totalSizeCloseRequest() - data.totalSizeAllocatedClose();
    size_t branchResultGain = data.totalSizeBranchResultRequest() - data.totalSizeAllocatedBranchResult();
    size_t branchGain = data.totalSizeBranch() - data.totalSizeAllocatedBranch();
    size_t arrGain = data.totalSizeRequestedArray - data.totalSizeAllocatedArray;
    out << "ResultSolution: total objec requested: " <<  data.totalCallSolution << " | total object allocated: " << data.totalAllocationSolution << " | size gain: " << solutionGain << "\n";
    out << "ResultClose : total objec requested: " <<  data.totalCallClose << " | total object allocated: " << data.totalAllocationClose << " | size gain: " << closeGain << "\n";
    out << "ResultBranch: total objec requested: " <<  data.totalCallBranchResult << " | total object allocated: " << data.totalAllocationBranchResult << " | size gain: " << branchResultGain << "\n";
    out << "Branch: total objec requested: " <<  data.totalCallBranch << " | total object allocated: " << data.totalAllocationBranch << " | size gain: " << branchGain << "\n";
    out << "Array of elem: total calloc requested: " <<  data.totalCallArrayElem << " | total object allocated: " << data.totalAllocationArray << " | size gain: " << arrGain << "\n";
    return out;
}

size_t KnapsackMemoryRecap::totalSizeBranchResultRequest() const {
    return sizeof(KnapsackResultBranch) * totalCallBranchResult;
 }

size_t KnapsackMemoryRecap::totalSizeAllocatedBranchResult() const {
    return sizeof(KnapsackResultBranch) * totalAllocationBranchResult;
}

size_t KnapsackMemoryRecap::totalSizeSolutionRequest() const {
    return sizeof(KnapsackResultSolution) * totalCallSolution;
 }

size_t KnapsackMemoryRecap::totalSizeAllocatedSolution() const {
    return sizeof(KnapsackResultSolution) * totalAllocationSolution;
}

size_t KnapsackMemoryRecap::totalSizeCloseRequest() const {
    return sizeof(KnapsackResultClose) * totalCallClose;
}

size_t KnapsackMemoryRecap::totalSizeAllocatedClose() const {
    return sizeof(KnapsackResultClose) * totalAllocationClose;
}

size_t KnapsackMemoryRecap::totalSizeBranch() const {
    return sizeof(KnapsackBranch) * totalCallBranch;
 }

size_t KnapsackMemoryRecap::totalSizeAllocatedBranch() const {
    return sizeof(KnapsackBranch) * totalAllocationBranch;
}
