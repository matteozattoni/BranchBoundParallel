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
    virtual eBranchBoundResultType getResultType() const =0;
};

class BranchBoundResultSolution: public BranchBoundResult
{
private:
    /* data */
public:
    eBranchBoundResultType getResultType() const override {return Solution;};
    virtual int getSolutionResult() const = 0;
    BranchBoundResultSolution() {}
    virtual ~BranchBoundResultSolution() {}
};

class BranchBoundResultBranch: public BranchBoundResult
{
private:
    Branch* branches;
    int numberOfBranch;
    /* data */
public:
    BranchBoundResultBranch(Branch* b,int n): branches(b), numberOfBranch(n) {}
    virtual ~BranchBoundResultBranch() {};
    eBranchBoundResultType getResultType() const override {return ResultBranch;};
    int getNumberBranch() const {return numberOfBranch;}
    Branch* getArrayBranch() const { return branches;}
};

class BranchBoundResultClosed: public BranchBoundResult
{
private:
    /* data */
public:
    BranchBoundResultClosed() {};
    virtual ~BranchBoundResultClosed() {};
    eBranchBoundResultType getResultType() const override {return Closed;};
};



#endif