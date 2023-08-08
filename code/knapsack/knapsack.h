#ifndef KNAPSACKCLASS
#define KNAPSACKCLASS

#include "knapsacklib.h"
#include "knapsacksolution.h"
#include "knapsacktask.h"
#include "../branchbound/result.h"
#include "knapsackresult.h"
#include "knapsackmemorymanager.h"


using namespace std;

class Knapsack {

private:
    int bound = -1;
    int problemDimension;
    int knapsackCapacity;
    bool isComputingSolution = false;
    KnapsackObject* problemElements;
    KnapsackSolution* currentSolution;
    void clearSolution();
    KnapsackMemoryManager* manager;
public:
    
    Knapsack(KnapsackObject* problemElements, int problemDimension, int knapsackCapacity);
    ~Knapsack();

    void setBound(int bound);

    void setCurrentTask(KnapsackTask* task);
    bool hasCurrentTask();
    void printCurrentSolution();

    BranchBoundResult* computeTaskIteration();


};


#endif