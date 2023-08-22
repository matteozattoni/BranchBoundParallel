#include "knapsackbranch.h"
#include <string.h>

#include <iostream>

AllocatorFixedMemoryPool<KnapsackBranch>* KnapsackBranch::branchMemoryManager = new AllocatorFixedMemoryPool<KnapsackBranch>();
AllocatorArrayMemoryPool<KnapsackBranchElement>* KnapsackBranch::elementsMemoryManager = new AllocatorArrayMemoryPool<KnapsackBranchElement>(0.4, 50);
const KnapsackBranch& KnapsackBranch::rootBranch = KnapsackBranch(0, nullptr);

std::ostream& KnapsackBranch::printKnapsackBranchMemory(std::ostream& out) {
    out << *KnapsackBranch::branchMemoryManager;
    out << "  *Array of KnapsackBranchElem" << std::endl;
    out << *KnapsackBranch::elementsMemoryManager;
    return out;
}

KnapsackBranchElement::KnapsackBranchElement(int id, bool isInsideKnapsack): BranchElement(id), insideKnapsack(isInsideKnapsack) {}

KnapsackBranchElement::~KnapsackBranchElement() {}

KnapsackBranch::KnapsackBranch(int numberElements, BranchElement* buffer): Branch(numberElements, buffer) {
    
}

KnapsackBranch::KnapsackBranch(int dimBuff, int numberElements, BranchElement* elementToCopy): Branch(numberElements, elementsMemoryManager->allocate(numberElements))
{
    if(dimBuff<numberElements) {
        throw OverflowArray;
    }
    
    KnapsackBranchElement* newBuffer =  static_cast<KnapsackBranchElement*>(this->elements);
    KnapsackBranchElement* fromElement = dynamic_cast<KnapsackBranchElement*>(elementToCopy);

    if (fromElement == nullptr || newBuffer == nullptr)
        throw InvalidBranchElementCast;

    for (int i = 0; i < numberElements; i++)
    {
        int id = fromElement[i].getElementId();
        bool inKnap = fromElement[i].isInsideKnapsack();
        new(&newBuffer[i]) KnapsackBranchElement(id, inKnap);
    }
    //memcpy(newBuffer, elementToCopy, sizeof(KnapsackBranchElement)*numberElements);
}

KnapsackBranch::~KnapsackBranch()
{
    elementsMemoryManager->deallocate(this->getKnapsackBranchElement());
}

KnapsackBranchElement *KnapsackBranch::getKnapsackBranchElement() const
{
    if (this->getNumberOfElements() > 0)
    {
        BranchElement *elem = this->getBranchElements();
        KnapsackBranchElement *knapsackElements = static_cast<KnapsackBranchElement *>(elem);
        if (knapsackElements == nullptr)
            throw InvalidBranchElementCast;
        return knapsackElements;
    }
    else
    {
        return nullptr;
    }
}

void* KnapsackBranch::operator new(size_t size) {
    void* ptr = KnapsackBranch::branchMemoryManager->allocate();
    return ptr;
}

void* KnapsackBranch::operator new(size_t size, void* ptr) {
    //void* ptr = KnapsackBranch::branchMemoryManager->allocate();
    return ptr;
}

void KnapsackBranch::operator delete(void* ptr) {
    KnapsackBranch::branchMemoryManager->deallocate((KnapsackBranch*)ptr);
}