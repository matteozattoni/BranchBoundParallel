#ifndef KNAPSACKSOLUTION
#define KNAPSACKSOLUTION

#include <set>
#include <utility>
#include "knapsacklib.h"

class KnapsackSolution
{
private:
    int problemSize;
    int solutionSize;
    int weigth = 0;
    int profit = 0;
    KnapsackObject* problemElements;
    KnapsackElementSolution* solutions;
    /* data */
public:
    void clear();
    KnapsackSolution(KnapsackObject* problemElements, int problemSize);
    ~KnapsackSolution();
    void copyElements(KnapsackElementSolution* elements, int size);
    void addObjectToSolution(int id, bool in_knapsack, int profit, int weigth);
    bool hasObjectId(int id);
    int getSolutionWeigth();
    int getSolutionProfit();
    int getSolutionSize();
    void printSolution();
    KnapsackElementSolution* getSolution();
};


#endif