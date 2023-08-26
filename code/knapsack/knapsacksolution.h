#ifndef KNAPSACKSOLUTION
#define KNAPSACKSOLUTION

#include <unordered_set>
#include <utility>
#include "knapsackproblem.h"
#include "knapsackbranch.h"

class KnapsackSolution
{
private:
    std::set<int> setElementsId;
    KnapsackProblem* problem;
    int numberSolutionElements = 0;
    int solutionTotalWeigth = 0;
    int solutionTotalProfit = 0;
    double solutionUpperbound = 0.0;
    KnapsackBranchElement* solutionElements;
    /* data */
public:
    void clear();
    KnapsackSolution(KnapsackProblem* problem);
    ~KnapsackSolution();
    void setElementsFromBranch(KnapsackBranch* branch);
    void addObjectToSolution(int id, bool in_knapsack, int profit, int weigth);
    bool hasObjectId(int id);
    void copySolutionTo(KnapsackBranchElement* buff);
    int getSolutionWeigth();
    int getSolutionProfit();
    int getSolutionSize();
    void printSolution();
    KnapsackBranchElement* getElementsSolution();
    void checkFeasibleSolution();
};


#endif