#ifndef KNAPSACKMEMORY
#define KNAPSACKMEMORY

#include <list>
#include <stdlib.h>
#include "knapsackresult.h"
#include "knapsacktask.h"

template<typename T>
class KnapsackFixedMemoryPool
{
private:
    std::list<T*> allocateList;
    /* data */
public:
    int numberOfMalloc = 0;
    T* allocate();

    void deallocate(T* ptr);

    KnapsackFixedMemoryPool<T>(/* args */);
    ~KnapsackFixedMemoryPool<T>();
};


template class KnapsackFixedMemoryPool<KnapsackResultBranch>;
template class KnapsackFixedMemoryPool<KnapsackResultSolution>;
template class KnapsackFixedMemoryPool<KnapsackTask>;

#endif