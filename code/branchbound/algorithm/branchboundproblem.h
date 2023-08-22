#ifndef BRANCHBOUNDPROBLEM_H
#define BRANCHBOUNDPROBLEM_H

#include <cstddef>

class BranchBoundProblemElement
{
private:
    /* data */
public:
    BranchBoundProblemElement() {}
    virtual ~BranchBoundProblemElement() {}
};


class BranchBoundProblem
{
protected:
    int problemElementsNumber;
    BranchBoundProblemElement * problemElements;
    /* data */
public:
    int getProblemElementsNumber() const {return problemElementsNumber;}
    BranchBoundProblemElement* getProblemElements() const { return problemElements;}
    BranchBoundProblem(BranchBoundProblemElement* el, int n) : problemElementsNumber(n), problemElements(el) {}
    virtual ~BranchBoundProblem() {}
};


#endif