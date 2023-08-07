#ifndef KNAPSACKMEMORY
#define KNAPSACKMEMORY

#include <list>
#include <stdlib.h>
#include "knapsackresult.h"

template<typename T>
class KnapsackResultMemoryPool
{
private:
    std::list<T*> allocateList;
    /* data */
public:
    int numberOfMalloc = 0;
    T* allocate();

    void deallocate(T* ptr);

    KnapsackResultMemoryPool<T>(/* args */);
    ~KnapsackResultMemoryPool<T>();
};


template class KnapsackResultMemoryPool<KnapsackResultBranch>;
template class KnapsackResultMemoryPool<KnapsackResultSolution>;

#endif