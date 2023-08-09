#include "knapsackmemory.h"

// Fixed

template <class T>
KnapsackFixedMemoryPool<T>::KnapsackFixedMemoryPool(/* args */)
{
}

template <class T>
KnapsackFixedMemoryPool<T>::~KnapsackFixedMemoryPool()
{
}

template <class T>
int KnapsackFixedMemoryPool<T>::getNumberOfMalloc()
{
    return numberOfMalloc;
}

template <class T>
T *KnapsackFixedMemoryPool<T>::allocate()
{
    if (allocateList.empty())
    {
        numberOfMalloc++;
        return (T *)malloc(sizeof(T));
    }
    else
    {
        T *ptr = allocateList.front();
        allocateList.pop_front();
        return ptr;
    }
}

template <class T>
void KnapsackFixedMemoryPool<T>::deallocate(T *ptr)
{
    allocateList.push_front(ptr);
}


// Array

template<class T>
KnapsackArrayMemoryPool<T>::KnapsackArrayMemoryPool() : useDelta(false) {
}

template<class T>
KnapsackArrayMemoryPool<T>::KnapsackArrayMemoryPool(double pivot, int fromsize) : useDelta(true) {
    this->pivot = pivot;
    this->fromsize = fromsize;
}

template<class T>
KnapsackArrayMemoryPool<T>::~KnapsackArrayMemoryPool(){

}

template<class T>
int KnapsackArrayMemoryPool<T>::getNumberAllocation() {
    return numberOfCalloc;
}

template<class T>
T* KnapsackArrayMemoryPool<T>::allocate(size_t size) {
    typename std::set<_ChunkArray*>::iterator it;
    it = freeArrays.begin();
    while (it != freeArrays.end())
    {
        _ChunkArray* c = *it;
        size_t sizeChunk = c->sizeMemoryHeap;
        if (sizeChunk >= size && (!useDelta || isSizeDisposable(sizeChunk, size))) {
            break;
        }
        it++;
    }
    if (it != freeArrays.end()) {
        _ChunkArray* c = *it;
        T* ptr = c->pointer;
        poolChunk.deallocate(c);
        freeArrays.erase(it);
        return ptr;
    }

    T* newPtr = (T*) calloc(size, sizeof(T));
    pointerSize[newPtr] = size;
    numberOfCalloc++;
    return newPtr;
}

template<class T>
void KnapsackArrayMemoryPool<T>::deallocate(T* ptr) {
    size_t size = pointerSize[ptr];
    _ChunkArray* newChunk = poolChunk.allocate();
    newChunk->pointer = ptr;
    newChunk->sizeMemoryHeap = size;
    freeArrays.insert(newChunk);
}

template<class T>
bool KnapsackArrayMemoryPool<T>::isSizeDisposable(size_t a, size_t b) {
    double pivot = this->pivot; //0. percet which can be wasted 
    if (a < b)
        return false;
    if (a < this->fromsize)
        return true;
    double A = a;
    double B = b;
    double relativeDelta = A - (A/B); // wasted size
    if (relativeDelta > pivot)
        return false;
    return true;
}

template<class T>
size_t KnapsackArrayMemoryPool<T>::getTotalSizeAllocated() {
    size_t totalSize = 0;

    for(auto const& pair: pointerSize) {
        totalSize+=pair.second;
    }
    return totalSize;

}