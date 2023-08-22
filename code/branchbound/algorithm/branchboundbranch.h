#ifndef BRANCHBOUNDBRANCH_H
#define BRANCHBOUNDBRANCH_H


class BranchElement
{
protected:
    int id;
    /* data */
public:
    BranchElement(int elementIdentifier): id(elementIdentifier){};
    int getElementId() const {return id;};
    virtual ~BranchElement() {};
};


class Branch
{
protected:
    int numberOfElement;
    BranchElement* elements;
public:
    int getNumberOfElements() const {return numberOfElement;}
    BranchElement* getBranchElements() const {return elements;}
    Branch(int nElements, BranchElement* el): numberOfElement(nElements){
        this->elements = el;
    };
    virtual ~Branch() {};
};

#endif