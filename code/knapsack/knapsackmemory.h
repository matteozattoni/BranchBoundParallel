#ifndef KNAPSACKMEMORY
#define KNAPSACKMEMORY

#include <list>
#include <stdlib.h>
#include "knapsackresult.h"
#include "knapsacktask.h"

template<typename T>
class KnapsackStaticMemoryPool
{
private:
    std::list<T*> allocateList;
    /* data */
public:
    int numberOfMalloc = 0;
    T* allocate();

    void deallocate(T* ptr);

    KnapsackStaticMemoryPool<T>(/* args */);
    ~KnapsackStaticMemoryPool<T>();
};


template class KnapsackStaticMemoryPool<KnapsackResultBranch>;
template class KnapsackStaticMemoryPool<KnapsackResultSolution>;
template class KnapsackStaticMemoryPool<KnapsackTask>;

#endif