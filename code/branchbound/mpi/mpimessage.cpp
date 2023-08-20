#include "mpimessage.h"

AllocatorFixedMemoryPool<MPIMessage>* MPIMessage::memoryManager = new AllocatorFixedMemoryPool<MPIMessage>();

MPIMessage::MPIMessage(void* buffer, int count, int tag) {
        this->buffer = buffer;
        this->count = count;
        this->tag = tag;

}

MPIMessage::~MPIMessage() {}

void * MPIMessage::operator new(size_t size) {
    return (void*)memoryManager->allocate();
}
void * MPIMessage::operator new(size_t size, void* ptr) {
    return ptr;
}
void MPIMessage::operator delete(void * p) {
    memoryManager->deallocate((MPIMessage*) p);
}