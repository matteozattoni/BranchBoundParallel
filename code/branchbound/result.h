#ifndef BRANCHBOUNDRESULT
#define BRANCHBOUNDRESULT

#include "branchboundbranch.h"

enum eBranchBoundResultType { Solution, ResultBranch, Closed };

class BranchBoundResult
{
private:
    /* data */
protected:
    eBranchBoundResultType resultType;
public:
    BranchBoundResult() {}
    ~BranchBoundResult() {}
    virtual eBranchBoundResultType getResultType()=0;
};

class BranchBoundResultSolution: public virtual BranchBoundResult
{
private:
    /* data */
public:
    eBranchBoundResultType getResultType() override {return Solution;};
    virtual int getSolutionResult() = 0;
    BranchBoundResultSolution() {}
    virtual ~BranchBoundResultSolution() {}
};

class BranchBoundResultBranch: public virtual BranchBoundResult
{
private:
    const Branch* branches;
    const int numberOfBranch;
    /* data */
public:
    BranchBoundResultBranch(Branch* b,int n): branches(b), numberOfBranch(n) {}
    virtual ~BranchBoundResultBranch() {};
    eBranchBoundResultType getResultType() override {return ResultBranch;};
    int getNumberBranch() {return numberOfBranch;}
    const Branch* getArrayBranch() { return branches;}
};

class BranchBoundResultClosed: public virtual BranchBoundResult
{
private:
    /* data */
public:
    BranchBoundResultClosed() {};
    virtual ~BranchBoundResultClosed() {};
    eBranchBoundResultType getResultType() override {return Closed;};
};



#endif