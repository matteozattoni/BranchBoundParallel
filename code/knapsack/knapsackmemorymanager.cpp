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
KnapsackResultBranch* KnapsackMemoryManager::allocateResultBranch() {
    numberCallAllocationBranch++;
    return this->branchMemoryPool.allocate();
}
KnapsackTask* KnapsackMemoryManager::allocateTask() {
    numberCallAllocationTask++;
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

KnapsackElementSolution* KnapsackMemoryManager::allocateArray(size_t size) {
    numberCallAllocationArray++;
    sizeRequestedForArray+=size;
    if (size <= 0)
        return nullptr;
   return this->arrayMemoryPool.allocate(size);
}

void KnapsackMemoryManager::deallocateArray(KnapsackElementSolution* ptr) {
    if (ptr != nullptr)
        this->arrayMemoryPool.deallocate(ptr);
}

KnapsackMemoryRecap* KnapsackMemoryManager::getMemoryInfo() {
    size_t totArr = this->arrayMemoryPool.getTotalSizeAllocated();
    int allSol = this->solutionMemoryPool.getNumberOfMalloc();
    int callSol = numberCallAllocationSolution;
    int allBra = this->branchMemoryPool.getNumberOfMalloc();
    int callBra = numberCallAllocationBranch;
    int allTask = this->taskMemoryPool.getNumberOfMalloc();
    int callTask = numberCallAllocationTask;
    int allArr = this->arrayMemoryPool.getNumberAllocation();
    int callArr = numberCallAllocationArray;
    KnapsackMemoryRecap* info = new KnapsackMemoryRecap(totArr, sizeRequestedForArray, allSol, callSol, allBra, callBra, allTask, callTask, allArr, callArr);
    return info;
}

KnapsackMemoryRecap::KnapsackMemoryRecap(size_t totArr, size_t reqArr, int allSol, int callSol, int allBra, int callBra, int allTask, int callTask, int allArr, int callArr) {
    this->totalSizeAllocatedArray = totArr;
    this->totalSizeRequestedArray = reqArr;
    this->totalAllocationSolution = allSol;
    this->totalCallSolution = callSol;
    this->totalCallBranch = callBra;
    this->totalAllocationBranch = allBra;
    this->totalCallTask = callTask;
    this->totalAllocationTask = allTask;
    this->totalCallArrayElem = callArr;
    this->totalAllocationArray = allArr;
}


std::ostream& operator <<(std::ostream &out, KnapsackMemoryRecap const& data) {
    size_t solutionGain = data.totalSizeSolutionRequest() - data.totalSizeAllocatedSolution();
    size_t branchGain = data.totalSizeBranchRequest() - data.totalSizeAllocatedBranch();
    size_t taskGain = data.totalSizeTaskRequest() - data.totalSizeAllocatedTask();
    size_t arrGain = data.totalSizeRequestedArray - data.totalSizeAllocatedArray;
    out << "ResultSolution: total objec requested: " <<  data.totalCallSolution << " | total object allocated: " << data.totalAllocationSolution << " | size gain: " << solutionGain << "\n";
    out << "ResultBranch: total objec requested: " <<  data.totalCallBranch << " | total object allocated: " << data.totalAllocationBranch << " | size gain: " << branchGain << "\n";
    out << "Task: total objec requested: " <<  data.totalCallTask << " | total object allocated: " << data.totalAllocationTask << " | size gain: " << taskGain << "\n";
    out << "Array of elem: total calloc requested: " <<  data.totalCallArrayElem << " | total object allocated: " << data.totalAllocationArray << " | size gain: " << arrGain << "\n";
    return out;
}

size_t KnapsackMemoryRecap::totalSizeBranchRequest() const {
    return sizeof(KnapsackResultBranch) * totalCallBranch;
 }

size_t KnapsackMemoryRecap::totalSizeAllocatedBranch() const {
    return sizeof(KnapsackResultBranch) * totalAllocationBranch;
}

size_t KnapsackMemoryRecap::totalSizeSolutionRequest() const {
    return sizeof(KnapsackResultSolution) * totalCallSolution;
 }

size_t KnapsackMemoryRecap::totalSizeAllocatedSolution() const {
    return sizeof(KnapsackResultSolution) * totalAllocationSolution;
}

size_t KnapsackMemoryRecap::totalSizeTaskRequest() const {
    return sizeof(KnapsackTask) * totalCallTask;
 }

size_t KnapsackMemoryRecap::totalSizeAllocatedTask() const {
    return sizeof(KnapsackTask) * totalAllocationTask;
}
