#include "knapsackbranch.h"

KnapsackBranchElement::KnapsackBranchElement(int id, bool isInsideKnapsack): BranchElement(id), insideKnapsack(isInsideKnapsack)
{
}

KnapsackBranchElement::~KnapsackBranchElement()
{
}

KnapsackBranch::KnapsackBranch(double bound, int numberElements, BranchElement* elements): Branch(bound, numberElements, elements)
{
}

KnapsackBranch::~KnapsackBranch()
{
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