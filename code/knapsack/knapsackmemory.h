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
public:
    T* allocate();
    int getNumberOfMalloc();
    void deallocate(T* ptr);
    KnapsackFixedMemoryPool(/* args */);
    ~KnapsackFixedMemoryPool();
};

template<class T>
class KnapsackArrayMemoryPool
{
protected:
    const bool useDelta;
    double pivot;
    int fromsize;
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
    bool isSizeDisposable(size_t a, size_t b);
    KnapsackFixedMemoryPool<_ChunkArray> poolChunk;
    static bool compareChunk(const _ChunkArray* c1, const _ChunkArray* c2) { return c1->sizeMemoryHeap < c2->sizeMemoryHeap; };
    std::set<_ChunkArray*, cmp> freeArrays;
public:
    T* allocate(size_t size);
    void deallocate(T* ptr);
    int getNumberAllocation();
    size_t getTotalSizeAllocated();
    KnapsackArrayMemoryPool();
    KnapsackArrayMemoryPool(double,int);
    ~KnapsackArrayMemoryPool();
};


template class KnapsackFixedMemoryPool<KnapsackResultBranch>;
template class KnapsackFixedMemoryPool<KnapsackResultSolution>;
template class KnapsackFixedMemoryPool<KnapsackTask>;

template class KnapsackArrayMemoryPool<KnapsackElementSolution>;


#endif