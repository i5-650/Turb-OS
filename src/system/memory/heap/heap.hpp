#pragma once

#include <lib/lock.hpp>
#include <stdint.h>
#include <stddef.h>

#define INIT_PAGES 512

struct HeapBlock{
    size_t size;
    bool free;
};

class Heap {
    private:
    HeapBlock *head = nullptr;
    HeapBlock *tail = nullptr;
    void *data = nullptr;
    bool expanded = false;
    lock_t lock;

    HeapBlock *next(HeapBlock *block);
    HeapBlock *split(HeapBlock *block, size_t size);

    HeapBlock *find_best(size_t size);
    size_t required_size(size_t size);
    void coalescence();

    public:
    bool debug = false;
    size_t pages = 0;

    void expand(size_t pagecount = 16);
    void setsize(size_t pagecount);

    void *malloc(size_t size);
    void *calloc(size_t num, size_t size);
    void *realloc(void *ptr, size_t size);
    void free(void *ptr);

    size_t allocsize(void *ptr);
};

extern Heap turboHeap;

static inline void *malloc(size_t size, bool calloc = true){
    if(calloc){
		return turboHeap.calloc(1, size);
	}
    return turboHeap.malloc(size);
}
static inline void *calloc(size_t num, size_t size){
    return turboHeap.calloc(num, size);
}
static inline void *realloc(void *ptr, size_t size){
    return turboHeap.realloc(ptr, size);
}
static inline void free(void *ptr){
    turboHeap.free(ptr);
}

static inline size_t allocsize(void *ptr){
    return turboHeap.allocsize(ptr);
}

void *operator new(size_t size);
void *operator new[](size_t size);

void operator delete(void *ptr);
void operator delete[](void *ptr);