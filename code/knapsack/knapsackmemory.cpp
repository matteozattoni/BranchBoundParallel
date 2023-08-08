#include "knapsackmemory.h"

template <typename T>
KnapsackFixedMemoryPool<T>::KnapsackFixedMemoryPool(/* args */)
{
}

template <typename T>
KnapsackFixedMemoryPool<T>::~KnapsackFixedMemoryPool()
{
}

template <typename T>
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

template <typename T>
void KnapsackFixedMemoryPool<T>::deallocate(T *ptr)
{
    allocateList.push_front(ptr);
}

