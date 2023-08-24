#ifndef KNAPSACKCLASS
#define KNAPSACKCLASS

#include <ostream>
#include <exception>

#include "../branchbound/algorithm/branchboundalgorithm.h"
#include "../branchbound/algorithm/branchboundproblem.h"
#include "../branchbound/algorithm/branchboundresult.h"
#include "../branchbound/algorithm/branchboundexception.h"
#include "../branchbound/branchbound.h"
#include "knapsacksolution.h"
#include "knapsackproblem.h"
#include "knapsackresult.h"


using namespace std;

class KnapsackException: public std::exception {
private:
    const std::string reason;
public:
    KnapsackException(std::string stringReason): reason(stringReason) {};
    ~KnapsackException() {}
    const char * what () const throw () {
        return reason.c_str();
    }
};

class KnapsackSubProblem
{
private:
    static AllocatorFixedMemoryPool<KnapsackSubProblem>* memoryManager;
    /* data */
public:
    const double upperbound;
    const double lowerbound;
    const int idObject;
    const bool foundObject;
    const double residualCapacity;
    KnapsackSubProblem(double upperbound, double lowerbound, int id, bool isFoundObject, double capacity);
    ~KnapsackSubProblem();
    void * operator new(size_t size);
    void operator delete(void * p);
};


class Knapsack: public BranchBoundAlgorithm {

private:
    int bound = -1;
    bool isComputingSolution = false;
    KnapsackSolution* currentSolution;
    void clearSolution();

    void setKnapsackProblem(KnapsackProblem*);
    KnapsackProblem* getKnapsackProblem();
    KnapsackSubProblem* computeSubSolution(KnapsackBranchElement* elem, int numb) const;
    KnapsackSubProblem* computeSubSolution() const;
public:
    Knapsack(/* manager?*/);
    ~Knapsack();

    bool isBetterBound(int bound) override;
    void setBound(int bound) override;
    void setProblem(BranchBoundProblem*) override;
    void setProblemWithRootBranch(BranchBoundProblem*) override;
    void setBranch(Branch*) override;
    BranchBoundResult* computeTaskIteration() override;
    bool hasCurrentBranch() override;
    std::ostream& printAlgorithm(std::ostream& out) override;

    void printCurrentSolution();
    friend std::ostream& operator <<(std::ostream &out, Knapsack const& data);
    void printMemoryInfo(std::ostream& out);

};


#endif