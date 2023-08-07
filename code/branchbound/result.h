#ifndef BRANCHBOUNDRESULT
#define BRANCHBOUNDRESULT

enum eBranchBoundResultType { Solution, Branch };

class BranchBoundResult
{
private:
    /* data */
protected:
    eBranchBoundResultType resultType;
public:
    BranchBoundResult(/* args */);
    ~BranchBoundResult();
    virtual eBranchBoundResultType getResultType() = 0;
};

class BranchBoundResultSolution: public virtual BranchBoundResult
{
private:
    /* data */
public:
    eBranchBoundResultType getResultType() {return Solution;};
    virtual int getSolutionResult() = 0;
    BranchBoundResultSolution();
    ~BranchBoundResultSolution();
};

class BranchBoundResultBranch: public virtual BranchBoundResult
{
private:
    /* data */
public:
    BranchBoundResultBranch();
    ~BranchBoundResultBranch();
    eBranchBoundResultType getResultType() {return Branch;};
    virtual int getNumberBranch() = 0;
    virtual void* getArrayBranch() = 0;
};


#endif