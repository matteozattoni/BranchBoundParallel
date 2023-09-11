#ifndef BRANCHBOUNDRESULT
#define BRANCHBOUNDRESULT

#include "branchboundbranch.h"
#include <list>

enum eBranchBoundResultType { Solution, Branches, Closed };

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
protected:
    const std::list<Branch*> branches;
    /* data */
public:
    BranchBoundResultBranch(std::list<Branch*> listOfBranch): branches(listOfBranch) {}
    virtual ~BranchBoundResultBranch() {};
    eBranchBoundResultType getResultType() const override {return Branches;};
    int getNumberBranch() const {return branches.size();}
    const std::list<Branch*> getListBranch() const { return branches;}
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