#ifndef MPIMESSAGE_H
#define MPIMESSAGE_H

#include "../algorithm/branchboundmemory_impl.h"

class MPIMessage {
public:
    static AllocatorFixedMemoryPool<MPIMessage>* memoryManager;
    void* buffer;
    int count;
    int tag;
    MPIMessage(void* buffer, int count, int tag);
    ~MPIMessage();
    void * operator new(size_t size);
    void * operator new(size_t size, void* ptr);
    void operator delete(void * p);
};


#endif