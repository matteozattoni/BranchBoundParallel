#ifndef KNAPSACKTASK
#define KNAPSACKTASK
#include "knapsacklib.h"
#include "knapsacksolution.h"
#include <string.h>

class KnapsackTask {

private:

public:
    int size;
    KnapsackTask(KnapsackElementSolution* ob, int size);
    KnapsackTask(KnapsackSolution* solution, int newElemId, bool inKnapsack);
    KnapsackElementSolution* objects;
    void setBuffer(KnapsackElementSolution* buff, int size);
    void copyFromSolution(KnapsackSolution* solution, int newElemId, bool inKnapsack);
    ~KnapsackTask();
};

#endif