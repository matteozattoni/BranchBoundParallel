#ifndef KNAPSACKMEMORY
#define KNAPSACKMEMORY

#include <list>
#include <map>
#include <stdlib.h>
#include "knapsackresult.h"
#include "knapsacktask.h"
#include "knapsacklib.h"

template<class T>
class KnapsackFixedMemoryPool
{
private:
    std::list<T*> allocateList;
    int numberOfMalloc = 0;
    /* data */
public:
    T* allocate();
    int getNumberAllocation();
    void deallocate(T* ptr);
    KnapsackFixedMemoryPool(/* args */);
    ~KnapsackFixedMemoryPool();
};

template<class T>
class KnapsackArrayMemoryPool
{
private:
    int numberOfCalloc = 0;
    std::map<T*, size_t> pointerSize;
    struct _ChunkArray
    {
        T* pointer;
        size_t sizeMemoryHeap;
    };
    struct cmp {
	bool operator()(const _ChunkArray* x, const _ChunkArray* y) const { return x->sizeMemoryHeap < y->sizeMemoryHeap; }
};
    KnapsackFixedMemoryPool<_ChunkArray> poolChunk;
    static bool compareChunk(const _ChunkArray* c1, const _ChunkArray* c2) { return c1->sizeMemoryHeap < c2->sizeMemoryHeap; };
    std::set<_ChunkArray*, cmp> freeArrays;
public:
    T* allocate(size_t size);
    void deallocate(T* ptr);
    int getNumberAllocation();
    KnapsackArrayMemoryPool(/* args */);
    ~KnapsackArrayMemoryPool();
};

template class KnapsackFixedMemoryPool<KnapsackResultBranch>;
template class KnapsackFixedMemoryPool<KnapsackResultSolution>;
template class KnapsackFixedMemoryPool<KnapsackTask>;

template class KnapsackArrayMemoryPool<KnapsackElementSolution>;


#endif