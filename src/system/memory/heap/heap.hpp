#pragma once 

#include <stdint.h>
#include <stddef.h>

namespace turbo::heap {

	#define BLOCK_SIZE 0x8

	// in C++, struct are class but with everything public and
	// aligned in memory + the functions aren't really stored in
	// the struct, that's why we will use struct here
	struct HeapSegmentHeader {
		size_t length;
		struct HeapSegmentHeader *nextSegment;
		struct HeapSegmentHeader *lastSegment;
		bool isFree;
		void mergeCurrentNextToLast();
		void mergeCurrentToLast();
		void mergeNextToCurrent();
		void split(size_t length); 
	};

	extern bool isInit;
	
	extern struct HeapSegmentHeader* head;
	extern struct HeapSegmentHeader* tail;

	extern size_t totalSize;
	extern size_t freeSize;
	extern size_t usedSize;

	extern void *heapEnd;

	void init(void* heapAddress = (void*)0x0000100000000000, size_t pageCount = 0x10);

	size_t getSize(void* ptr);

	void *malloc(size_t size);
	void *calloc(size_t m, size_t n);
	void *realloc(void* ptr, size_t size);
	void free(void* address);

	void expandHeap(size_t length);
}

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void* ptr);