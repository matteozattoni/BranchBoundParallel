#include "knapsackbranch.h"
#include <string.h>

AllocatorFixedMemoryPool<KnapsackBranch>* KnapsackBranch::branchMemoryManager = new AllocatorFixedMemoryPool<KnapsackBranch>();
AllocatorArrayMemoryPool<KnapsackBranchElement>* KnapsackBranch::elementsMemoryManager = new AllocatorArrayMemoryPool<KnapsackBranchElement>(0.2, 50);

std::ostream& KnapsackBranch::printKnapsackBranchMemory(std::ostream& out) {
    out << *KnapsackBranch::branchMemoryManager;
    out << "  *Array of KnapsackBranchElem" << std::endl;
    out << *KnapsackBranch::elementsMemoryManager;
    return out;
}

KnapsackBranchElement::KnapsackBranchElement(int id, bool isInsideKnapsack): BranchElement(id), insideKnapsack(isInsideKnapsack) {}

KnapsackBranchElement::~KnapsackBranchElement() {}

KnapsackBranch::KnapsackBranch(double bound, int dimBuff, int numberElements, BranchElement* elementToCopy): Branch(bound, numberElements, elementsMemoryManager->allocate(dimBuff)), bufferDimension(dimBuff)
{
    if(dimBuff<numberElements) {
        throw OverflowArray;
    }
        
    KnapsackBranchElement* newBuffer = (KnapsackBranchElement*) this->elements;
    memcpy(newBuffer, elementToCopy, sizeof(KnapsackBranchElement)*numberElements);
}

KnapsackBranch::~KnapsackBranch()
{
    elementsMemoryManager->deallocate(this->getKnapsackBranchElement());
}

const KnapsackBranchElement *KnapsackBranch::getKnapsackBranchElement() const
{
    if (this->getNumberOfElements() > 0)
    {
        const BranchElement *elem = this->getBranchElements();
        const KnapsackBranchElement *knapsackElements = dynamic_cast<const KnapsackBranchElement *>(elem);
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

void KnapsackBranch::operator delete(void* ptr) {
    KnapsackBranch::branchMemoryManager->deallocate((KnapsackBranch*)ptr);
}