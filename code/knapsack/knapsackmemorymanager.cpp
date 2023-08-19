#include "knapsackmemorymanager.h"
#include <ostream>
#include <iostream>
#include <cassert>


using namespace std;

// KnapsackMemoryManager* KnapsackMemoryManager::singleton = new KnapsackMemoryManager();

KnapsackMemoryManager::KnapsackMemoryManager(/* args */)
{
}

KnapsackMemoryManager::~KnapsackMemoryManager()
{
}

const Branch* KnapsackMemoryManager::getRootBranch() {
    return &KnapsackBranch::rootBranch;
}

// create datatype

void KnapsackMemoryManager::createProblemDatatype() {
    struct {
        void* ptrVirtualTable;
        int totalNumberOfElements;
        void *ptrArrayElement;
        int knapCapacity;
    } problemStruct;
    int numberOfBlocks = 2;
    int numberOfElementPerBlock[] = {1,1};
    MPI_Datatype arrayOfType[] = {MPI_INT, MPI_INT};
    MPI_Aint startProblemStruct;
    MPI_Aint numberOfDisplamentsPerBlocs[2];
    MPI_Get_address(&problemStruct, &startProblemStruct);
    MPI_Get_address(&problemStruct.totalNumberOfElements, &numberOfDisplamentsPerBlocs[0]);
    MPI_Get_address(&problemStruct.knapCapacity, &numberOfDisplamentsPerBlocs[1]);
    numberOfDisplamentsPerBlocs[1] -= startProblemStruct;
    numberOfDisplamentsPerBlocs[0] -= startProblemStruct;
    MPI_Type_create_struct(numberOfBlocks, numberOfElementPerBlock, numberOfDisplamentsPerBlocs, arrayOfType, &problemDatatype);
}

void KnapsackMemoryManager::createProblemElementDatatype() {
    struct
    {
        void *ptrVirtualTable;
        double profit;
        double weigth;

    } problemElementStruct;
    int numberOfBlocks = 1;
    int numberOfElementPerBlock[] = {2};
    MPI_Datatype arrayOfType[] = {MPI_DOUBLE};
    MPI_Aint startProblemElementStruct;
    MPI_Aint numberOfDisplamentsPerBlocs[1];
    MPI_Get_address(&problemElementStruct, &startProblemElementStruct);
    MPI_Get_address(&problemElementStruct.profit, &numberOfDisplamentsPerBlocs[0]);
    numberOfDisplamentsPerBlocs[0] -= startProblemElementStruct;
    MPI_Type_create_struct(numberOfBlocks, numberOfElementPerBlock, numberOfDisplamentsPerBlocs, arrayOfType, &problemElementDatatype);
}

void KnapsackMemoryManager::createBoundDatatype() {
    struct
    {
        void *a;
        int solution;
        void *c, *b;

    } boundStruct;
    int numberOfBlocks = 1;
    int numberOfElementPerBlock[] = {1};
    MPI_Datatype arrayOfType[] = {MPI_INT};
    MPI_Aint startBoundStruct;
    MPI_Aint numberOfDisplamentsPerBlocs[1];
    MPI_Get_address(&boundStruct, &startBoundStruct);
    MPI_Get_address(&boundStruct.solution, &numberOfDisplamentsPerBlocs[0]);
    numberOfDisplamentsPerBlocs[0] -= startBoundStruct;
    MPI_Type_create_struct(numberOfBlocks, numberOfElementPerBlock, numberOfDisplamentsPerBlocs, arrayOfType, &boundDatatype);
}

void KnapsackMemoryManager::createBranchDatatype() {
    struct
    {
        void *ptrVirtualTable;
        int idElement;
        bool isInKnapsack;
    } branchElementStruct;
    size_t sizeStruct = sizeof(branchElementStruct);
    int numberOfBlocks = 2;
    int numberOfElementPerBlock[] = {1,1};
    MPI_Datatype arrayOfType[] = {MPI_INT, MPI_C_BOOL};
    MPI_Aint startBranchElementStruct;
    MPI_Aint numberOfDisplamentsPerBlocs[2];
    MPI_Get_address(&branchElementStruct, &startBranchElementStruct);
    MPI_Get_address(&branchElementStruct.idElement, &numberOfDisplamentsPerBlocs[0]);
    MPI_Get_address(&branchElementStruct.isInKnapsack, &numberOfDisplamentsPerBlocs[1]);
    numberOfDisplamentsPerBlocs[1] -= startBranchElementStruct;
    numberOfDisplamentsPerBlocs[0] -= startBranchElementStruct;
    MPI_Type_create_struct(numberOfBlocks, numberOfElementPerBlock, numberOfDisplamentsPerBlocs, arrayOfType, &branchDatatype);
}


// Datatype

MPI_Datatype KnapsackMemoryManager::getProblemType() { return problemDatatype; }

MPI_Datatype KnapsackMemoryManager::getProblemElementType() { return problemElementDatatype; }

MPI_Datatype KnapsackMemoryManager::getBoundType() { return boundDatatype; }

MPI_Datatype KnapsackMemoryManager::getBranchType() { return branchDatatype; }

void KnapsackMemoryManager::commitDatatypes() {
    createProblemDatatype();
       
    createProblemElementDatatype();
    createBoundDatatype();
    createBranchDatatype();

    MPI_Type_commit(&problemDatatype);
    MPI_Type_commit(&problemElementDatatype);
    MPI_Type_commit(&boundDatatype);
    MPI_Type_commit(&branchDatatype);
    struct problemElementStruct
    {
        void *ptrVirtualTable;
        double profit, weigth;

    };
    struct branchElementStruct
    {
        void *ptrVirtualTable;
        int idElement;
        bool isInKnapsack;
    };
    struct boundStruct
    {
        void *a;
        int solution;
        void *c, *b;

    };
    MPI_Datatype notResizedDatatype;
    MPI_Aint lb, ext;

    notResizedDatatype = problemElementDatatype;
    MPI_Type_get_extent(problemElementDatatype, &lb, &ext);
    MPI_Type_create_resized(problemElementDatatype, lb, sizeof(problemElementStruct), &problemElementDatatype);
    MPI_Type_free(&notResizedDatatype);
    notResizedDatatype = branchDatatype;
    MPI_Type_get_extent(branchDatatype, &lb, &ext);
    MPI_Type_create_resized(branchDatatype, lb, sizeof(branchElementStruct), &branchDatatype);
    MPI_Type_free(&notResizedDatatype);
    notResizedDatatype = boundDatatype;
    MPI_Type_get_extent(boundDatatype, &lb, &ext);
    MPI_Type_create_resized(boundDatatype, lb, sizeof(boundStruct), &boundDatatype);
    MPI_Type_free(&notResizedDatatype);
    MPI_Type_get_extent(boundDatatype, &lb, &ext);
    assert(sizeof(boundStruct) == ext);
}

// Problem

BranchBoundProblem* KnapsackMemoryManager::getLocalProblem() const {
    return KnapsackProblem::problemFromFile(FILEPATH);
}

void* KnapsackMemoryManager::getProblemTypeBuffFrom(BranchBoundProblem* problem) {
    return problem;
}

std::pair<void*,int> KnapsackMemoryManager::getProblemElementBuffFrom(BranchBoundProblem* problem) {
    return std::pair<void*,int>((void*)problem->getProblemElements(), problem->getProblemElementsNumber());
}

void* KnapsackMemoryManager::getEmptyProblemTypeBuff() {
    return malloc(sizeof(KnapsackProblem));
}

std::pair<void*,int> KnapsackMemoryManager::getEmptyProblemElementBuffFromType(void* problemType) {
    struct problemStruct{
        void* ptrVirtualTable;
        int totalNumberOfElements;
        void *ptrArrayElement;
        int knapCapacity;
    };
    int totalNumberOfElements = ((problemStruct*)problemType)->totalNumberOfElements;
    void* elementsBuffer = calloc(totalNumberOfElements, sizeof(KnapsackProblemElement));
    return std::pair<void*,int>(elementsBuffer, totalNumberOfElements);
}

BranchBoundProblem* KnapsackMemoryManager::getRemoteProblem(void* problemType, std::pair<void*,int> problemElements) {
    struct problemStruct{
        void* ptrVirtualTable;
        int totalNumberOfElements;
        void *ptrArrayElement;
        int knapCapacity;
    };
    struct problemElementStruct
    {
        void *ptrVirtualTable;
        double profit, weigth;

    };
    int totalNumberOfElements = ((problemStruct*)problemType)->totalNumberOfElements;
    int knapCapacity = ((problemStruct*)problemType)->knapCapacity;
    KnapsackProblemElement* elementsBuffer = (KnapsackProblemElement*) problemElements.first;
    int count = problemElements.second;
        
    for (int i = 0; i < count; i++)
    { 
        int profit = ((KnapsackProblemElement*)&elementsBuffer[i])->profit;
        int weigth = ((KnapsackProblemElement*)&elementsBuffer[i])->weight;
        new(&elementsBuffer[i]) KnapsackProblemElement(profit, weigth);
    }
    return new KnapsackProblem(elementsBuffer, count, knapCapacity);

}

// BOUND

std::pair<void*,int> KnapsackMemoryManager::getBoundBuffer(const BranchBoundResultSolution* solution) {
    void* ptr = (void*) solution;
    return std::pair<void*,int>(ptr,1);
}

void* KnapsackMemoryManager::getEmptybBoundBuff() {
    void* ptr = KnapsackResultSolution::memoryManager->allocate();
    return ptr;
}

BranchBoundResultSolution* KnapsackMemoryManager::getSolutionFromBound(void* buff) {
    struct boundStruct
    {
        void *a;
        int solution;
        void *c, *b;

    };
    int solution = ((boundStruct*) buff)->solution;
    //cout << "mem " << solution << " other " << ((boundStruct*) buff)->a << " " << ((boundStruct*) buff)->c << " " << ((boundStruct*) buff)->b << endl;
    KnapsackResultSolution* solPtr = (KnapsackResultSolution*) buff;
    new(solPtr) KnapsackResultSolution(solution);
    return solPtr;
}

// RECV
 void* KnapsackMemoryManager::getEmptyBranchElementBuff(int count) {
    void* ptr = KnapsackBranch::elementsMemoryManager->allocate(count);
    return ptr;
 }

 // SEND
std::pair<void*,int> KnapsackMemoryManager::getBranchBuffer(const Branch* branch) {
    int count = branch->getNumberOfElements();
    void* ptr = (void*) branch->getBranchElements();
    return std::pair(ptr,count);
}

void KnapsackMemoryManager::sentFinished(void* buff, int count) {
    KnapsackBranchElement* elements = (KnapsackBranchElement*) buff;
    delete elements;
}

BranchBoundResultBranch* KnapsackMemoryManager::getBranchFromBuff(void* buff, int count) {
    struct branchElementStruct
    {
        void *ptrVirtualTable;
        int idElement;
        bool isInKnapsack;
    };

    branchElementStruct* ptr = (branchElementStruct*) buff;
    for (int i = 0; i < count; i++)
    {
        int idElement = ptr[i].idElement;
        bool isInElement = ptr[i].isInKnapsack;
        new(&ptr[i]) KnapsackBranchElement(idElement, isInElement);
    }
    KnapsackBranchElement* elements = (KnapsackBranchElement*) buff;
    KnapsackBranch* branch = new KnapsackBranch(count, elements);
    return new KnapsackResultBranch(branch, 1);
}


void KnapsackMemoryManager::testProblemMemory()
{
    // TEST KnapsackProblem Memory Layout
    struct
    {
        void *ptr;
        double p, w;

    } testProbElem[2];
    testProbElem[0].p = 0.2;
    testProbElem[0].w = 0.6;
    testProbElem[1].p = 0.8;
    testProbElem[1].w = 0.11;
    assert(sizeof(testProbElem[0]) == sizeof(KnapsackProblemElement));
    KnapsackProblemElement *tPE = (KnapsackProblemElement *)&testProbElem;
    assert(tPE[0].profit == 0.2);
    assert(tPE[0].weight == 0.6);
    assert(tPE[1].profit == 0.8);
    assert(tPE[1].weight == 0.11);
    struct {
        void* virtuas;
        int numEl;
        void *arr;
        int knapCap;
    } testProblem;
    assert(sizeof(testProblem)==sizeof(KnapsackProblem));
    new(&tPE[0]) KnapsackProblemElement(tPE[0].profit, tPE[0].weight);
    new(&tPE[1]) KnapsackProblemElement(tPE[1].profit, tPE[1].weight);
    testProblem.numEl = 3;
    testProblem.arr = tPE;
    testProblem.knapCap = 5;
    KnapsackProblem *P = (KnapsackProblem*) &testProblem;
    new(P) KnapsackProblem(tPE, testProblem.numEl, testProblem.knapCap);
    assert(P->getProblemElementsNumber() == 3);
    assert(P->getTotalKnapsackCapacity() == 5);
    assert(P->getKnapsackProblemElements()[0].profit == tPE[0].profit);
    assert(P->getKnapsackProblemElements()[0].weight == tPE[0].weight);
    assert(P->getKnapsackProblemElements()[1].profit == tPE[1].profit);
    assert(P->getKnapsackProblemElements()[1].weight == tPE[1].weight);
}

void KnapsackMemoryManager::testBoundMemory() {
    // TEST KnapsackBranch Memory Layout
    struct BoundStruct
    {
        void *a;
        int solution;
        void *c, *b;

    } boundStruct;
    boundStruct.solution = 21;
    assert(sizeof(boundStruct)==sizeof(KnapsackResultSolution));
    KnapsackResultSolution* kb = (KnapsackResultSolution*)&boundStruct;
    new(kb) const KnapsackResultSolution(boundStruct.solution);
    assert(kb->getSolutionResult() == 21);
    KnapsackResultSolution newKb(50);
    int solution = ((BoundStruct*) &newKb)->solution;
    assert (solution == 50);
}

void KnapsackMemoryManager::testBranchMemory() {
    // TEST KnapsackBranch Memory Layout
    struct
    {
        void *stubVirtual;
        int id;
        bool in;
    } testBranchElem[2];
    testBranchElem[0].id = 1;
    testBranchElem[0].in = true;
    testBranchElem[1].id = 2;
    testBranchElem[1].in = false;
    assert(sizeof(testBranchElem[0])==sizeof(KnapsackBranchElement));
    KnapsackBranchElement* kb = (KnapsackBranchElement*)&testBranchElem;
    assert(kb[0].getElementId() == 1);
    assert(kb[0].isInsideKnapsack() == true);
    assert(kb[1].getElementId() == 2);
    assert(kb[1].isInsideKnapsack() == false);
    new(&kb[0]) KnapsackBranchElement(testBranchElem[0].id, testBranchElem[0].in);
    new(&kb[1]) KnapsackBranchElement(testBranchElem[1].id, testBranchElem[1].in);
    assert(kb[0].getElementId() == 1);
    assert(kb[0].isInsideKnapsack() == true);
    assert(kb[1].getElementId() == 2);
    assert(kb[1].isInsideKnapsack() == false);
    struct
    {
        void* stubVirtual;
        int buffDim;
        void* elem;
        int num;
        /* data */
    } testBranch;
    testBranch.buffDim = 2;
    testBranch.elem = kb;
    testBranch.num = 2;
    KnapsackBranch* b = (KnapsackBranch*) &testBranch;
    new(b) KnapsackBranch(2,2,kb);
    assert(sizeof(testBranch) == sizeof(KnapsackBranch));
    assert(b->getNumberOfElements() == 2);
    assert(b->getBranchElements()[0].getElementId() == 1);
    assert(b->getBranchElements()[1].getElementId() == 2);
    assert(b->getKnapsackBranchElement()[0].isInsideKnapsack() == true);
}

void KnapsackMemoryManager::test2()
{
    class A
    {
    public:
        virtual void printA() = 0;
    };
    class B : public A
    {
    private:
        double b;

    public:
        double getB() { return b; }
        B(){};
        void printA() override { cout << "A" << endl; }
    };
    struct
    {
        void *pt;
        double x;
    } bs;

    struct
    {
        void *ptr;
        double p, w;

    } testProbElem;
    testProbElem.p = 0.2;
    testProbElem.w = 0.6;
    KnapsackProblemElement *tPE = (KnapsackProblemElement *)&testProbElem;
    assert(tPE->profit == 0.2);
    assert(tPE->weight == 0.6);
    bs.x = 5.5;
    B *bsB = (B *)&bs;
    new (bsB) B();
    cout << bsB->getB() << endl;
    assert(bsB->getB() == 5.5);
    bsB->printA();
}