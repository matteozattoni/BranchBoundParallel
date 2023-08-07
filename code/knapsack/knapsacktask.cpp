#include "knapsacktask.h"

KnapsackTask::KnapsackTask(KnapsackElementSolution* solutions, int size)
{
    this->objects = solutions;
    this->size = size;
}

KnapsackTask::~KnapsackTask()
{
}
