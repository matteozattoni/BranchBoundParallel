#ifndef KNAPSACKTASK
#define KNAPSACKTASK
#include "knapsacklib.h"

class KnapsackTask {

private:
    
public:
    int size;
    KnapsackElementSolution* objects;
    //KnapsackSolution* solutions;
    KnapsackTask(KnapsackElementSolution* solutions, int size);
    ~KnapsackTask();
    static KnapsackElementSolution* getTaskBuffer(int dimension) {
        return (KnapsackElementSolution*) calloc(dimension, sizeof(KnapsackElementSolution));
    }
};

#endif