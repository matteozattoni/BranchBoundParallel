#ifndef KNAPSACKMEMORY
#define KNAPSACKMEMORY

#include <list>
#include <set>
#include <map>
#include <stdlib.h>
#include <ostream>

template <class T>
class AllocatorFixedMemoryPool
{
private:
    std::list<T *> allocateList;
    int numberOfMalloc = 0;
    int numberOfRequest = 0;

public:
    T *allocate()
    {
        numberOfRequest++;
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
    int getNumberOfMalloc() const
    {
        return numberOfMalloc;
    }

    int getNumberOfRequest() const{
        return numberOfRequest;
    }

    void deallocate(T *ptr)
    {
        allocateList.push_front(ptr);
    }
    friend std::ostream& operator <<(std::ostream &out, AllocatorFixedMemoryPool<T> const& data) {
        size_t potentialSizeAllocated = sizeof(T) * data.getNumberOfRequest();
        size_t realSizeAllocated = sizeof(T) * data.getNumberOfMalloc();
        //out<<"AllocatorFixedMemoryPool: " << typeid(data).name() <<" - " << std::endl;
        out<<"\tNumber of allocate function call: " << data.getNumberOfRequest() << std::endl;
        out<<"\tNumber of malloc called: " << data.getNumberOfMalloc() << std::endl;
        out<<"\tTotal Bytes requested: " << potentialSizeAllocated << std::endl;
        out<<"\tTotal Bytes allocated: " << realSizeAllocated << std::endl;
        return out;
    }
    AllocatorFixedMemoryPool(/* args */) {}
    ~AllocatorFixedMemoryPool() {}
};

template <class T>
class AllocatorArrayMemoryPool
{
protected:
    const bool useDelta;
    double pivot;
    int fromsize;
    size_t sizeAllocated = 0;
    size_t sizeRequested = 0;
    std::map<T *, size_t> pointerSize;
    struct _ChunkArray
    {
        T *pointer;
        size_t sizeMemoryHeap;
    };
    struct cmp
    {
        bool operator()(const _ChunkArray *x, const _ChunkArray *y) const { return x->sizeMemoryHeap < y->sizeMemoryHeap; }
    };
    bool isSizeDisposable(int a, int b)
    {
        double pivot = this->pivot; // 0. percet which can be wasted
        if (a < b)
            return false;
        if (a < this->fromsize)
            return true;
        double A = a;
        double B = b;
        double relativeDelta = (A - B)/A; // wasted size
        if (relativeDelta > pivot)
            return false;
        return true;
    }
    AllocatorFixedMemoryPool<_ChunkArray> poolChunk;
    static bool compareChunk(const _ChunkArray *c1, const _ChunkArray *c2) { return c1->sizeMemoryHeap < c2->sizeMemoryHeap; };
    std::set<_ChunkArray *, cmp> freeArrays;

public:
    T *allocate(size_t size)
    {
        sizeRequested+=size;
        typename std::set<_ChunkArray *>::iterator it;
        it = freeArrays.begin();
        while (it != freeArrays.end())
        {
            _ChunkArray *c = *it;
            size_t sizeChunk = c->sizeMemoryHeap;
            if (sizeChunk >= size && (!useDelta || isSizeDisposable(sizeChunk, size)))
            {
                break;
            }
            it++;
        }
        if (it != freeArrays.end())
        {
            _ChunkArray *c = *it;
            T *ptr = c->pointer;
            poolChunk.deallocate(c);
            freeArrays.erase(it);
            return ptr;
        }

        T *newPtr = (T *)calloc(size, sizeof(T));
        pointerSize[newPtr] = size;
        sizeAllocated+=size;
        return newPtr;
    }
    void deallocate(const T *pt)
    {
        T* ptr = (T*)pt;
        size_t size = pointerSize[ptr];
        _ChunkArray *newChunk = poolChunk.allocate();
        newChunk->pointer = ptr;
        newChunk->sizeMemoryHeap = size;
        freeArrays.insert(newChunk);
    }
    size_t getTotalSizeAllocated() const
    {
        size_t totalSize = 0;

        for (auto const &pair : pointerSize)
        {
            totalSize += pair.second;
        }
        return totalSize;
    }
    size_t getTotalSizeRequested() const { return sizeRequested;}
    friend std::ostream& operator <<(std::ostream &out, AllocatorArrayMemoryPool<T> const& data) {
        size_t potentialSizeAllocated = data.getTotalSizeRequested();
        size_t realSizeAllocated = data.getTotalSizeAllocated();
        //out<<"Class Name: " << typeid(data).name() <<" - " << std::endl;
        out<<"\tTotal Bytes requested: " << potentialSizeAllocated << std::endl;
        out<<"\tTotal Bytes allocated: " << realSizeAllocated << std::endl;
        return out;
    }
    /**
     * Will return an allocator for arrays which the function allocate(size) will return an unused
     * array with size greather or equals of the size requested, regardless of the actual size that will
     * be not used.
    */
    AllocatorArrayMemoryPool() : useDelta(false) {}
    /**
     * Will return an allocator for arrays which the function allocate(size) will only return an unused
     * array if the size is grather or equals to the size requested and if the size requested
     * is greather or equals than fromsize then the relative delta (between their size) must be less or 
     * equals than the pivot.
     * @param pivot ss
     * @param fromsize request which size is less then fromsize will 
    */
    AllocatorArrayMemoryPool(double pivot, int fromsize) : useDelta(true)
    {
        this->pivot = pivot;
        this->fromsize = fromsize;
    }
    ~AllocatorArrayMemoryPool() {}
};

#endif