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
    const int numberOfElement;
    const BranchElement* elements;
public:
    int getNumberOfElements() const {return numberOfElement;}
    const BranchElement* getBranchElements() const {return elements;}
    Branch(int nElements, BranchElement* el): numberOfElement(nElements), elements(el) {};
    virtual ~Branch() {};
};

#endif