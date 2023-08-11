#ifndef BRANCHBOUNDBRANCH_H
#define BRANCHBOUNDBRANCH_H


class BranchElement
{
protected:
    const int id;
    /* data */
public:
    BranchElement(int elementIdentifier): id(elementIdentifier){};
    int getElementId() const {return id;};
    virtual ~BranchElement() {};
};


class Branch
{
protected:
    const double branchBound;
    const int numberOfElement;
    const BranchElement* elements;
public:
    double getBranchBound() const {return branchBound;}
    int getNumberOfElements() const {return numberOfElement;}
    const BranchElement* getBranchElements() const {return elements;}
    Branch(double bound, int nElements, BranchElement* el): branchBound(bound), numberOfElement(nElements), elements(el) {};
    virtual ~Branch() {};
};

#endif