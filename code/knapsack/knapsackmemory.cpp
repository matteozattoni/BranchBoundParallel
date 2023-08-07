#include "knapsackmemory.h"

template <typename T>
KnapsackResultMemoryPool<T>::KnapsackResultMemoryPool(/* args */)
{
}

template <typename T>
KnapsackResultMemoryPool<T>::~KnapsackResultMemoryPool()
{
}

template <typename T>
T *KnapsackResultMemoryPool<T>::allocate()
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

template <typename T>
void KnapsackResultMemoryPool<T>::deallocate(T *ptr)
{
    allocateList.push_front(ptr);
}

