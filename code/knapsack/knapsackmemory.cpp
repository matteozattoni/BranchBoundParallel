#include "knapsackmemory.h"

template <typename T>
KnapsackStaticMemoryPool<T>::KnapsackStaticMemoryPool(/* args */)
{
}

template <typename T>
KnapsackStaticMemoryPool<T>::~KnapsackStaticMemoryPool()
{
}

template <typename T>
T *KnapsackStaticMemoryPool<T>::allocate()
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
void KnapsackStaticMemoryPool<T>::deallocate(T *ptr)
{
    allocateList.push_front(ptr);
}

