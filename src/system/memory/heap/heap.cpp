#include <drivers/display/serial/serial.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <system/memory/heap/heap.hpp>
#include <lib/memory/memory.hpp>
#include <lib/lock.hpp>

using namespace turbo;

namespace turbo::heap {
	bool isInit = false;

	struct HeapSegmentHeader* head;
	struct HeapSegmentHeader* tail;

	size_t totalSize;
	size_t freeSize;
	size_t usedSize;

	void *heapEnd;

	DEFINE_LOCK(heap_lock)

	void init(void* heapAddress, size_t pageCount){
		serial::log("[+] Initialising Heap\n");

		if(isInit){
			serial::log("[!!] Already init: Heap\n");
			return;
		}

		heapEnd = heapAddress;
		expandHeap(pageCount * 0x1000);

		serial::newline();
		isInit = true;
	}

	void hasBeenInit(){
		if(!isInit){
			serial::log("[/!\\] Heap not init\n");
			init();
		}
	}

	void free(void *address){
		hasBeenInit();

		if(!address){
			return;
		}

		acquire_lock(heap_lock);

		struct HeapSegmentHeader* header = (struct HeapSegmentHeader*)((uint64_t)address - sizeof(struct HeapSegmentHeader));
		header->isFree = true;
		freeSize += header->length + sizeof(struct HeapSegmentHeader);
		usedSize -= header->length + sizeof(struct HeapSegmentHeader);

		if(header->nextSegment && header->lastSegment){
			if(header->nextSegment->isFree && header->lastSegment->isFree){
				header->mergeCurrentNextToLast();
			}
		}
		else if(header->lastSegment){
			if(header->lastSegment->isFree){
				header->mergeCurrentToLast();
			}
		}
		else if(header->nextSegment){
			if(header->nextSegment->isFree){
				header->mergeNextToCurrent();
			}
		}

		//serial::log("[RAM] Freeing %zu bytes\n", header->length + sizeof(struct HeapSegmentHeader));

		release_lock(heap_lock);
	}

	void* malloc(size_t size){
		hasBeenInit();

		acquire_lock(heap_lock);

		if(size > pMemory::getFreeRam()){
			serial::log("[!!] malloc: can't alloc this much\n");
			return nullptr;
		}

		if(size % BLOCK_SIZE){
			size -= (size % BLOCK_SIZE);
			size += BLOCK_SIZE;
		}

		if(!size){
			return nullptr;
		}

		struct HeapSegmentHeader* currentSegment = reinterpret_cast<struct HeapSegmentHeader*>(head);

		while(true){

			if(currentSegment->isFree){
				if(currentSegment->length > size){
					currentSegment->split(size);
					currentSegment->isFree = false;

					usedSize += currentSegment->length + sizeof(struct HeapSegmentHeader);
					freeSize -= currentSegment->length + sizeof(struct HeapSegmentHeader);

					//serial::log("[RAM] Allocating %zu bytes\n", size + sizeof(struct HeapSegmentHeader));
					release_lock(heap_lock);

					return (void*)((uint64_t)currentSegment + sizeof(struct HeapSegmentHeader));
				}

				if(currentSegment->length == size){
					currentSegment->isFree = false;

					//serial::log("[RAM] Allocation %zu byes\n", size + sizeof(struct HeapSegmentHeader));

					usedSize += currentSegment->length + sizeof(struct HeapSegmentHeader);
					freeSize -= currentSegment->length + sizeof(struct HeapSegmentHeader);

					release_lock(heap_lock);

					return (void*)((uint64_t)currentSegment + sizeof(struct HeapSegmentHeader));
				}
			}

			if(!currentSegment->nextSegment){
				break;
			}

			currentSegment = currentSegment->nextSegment;
		}

		expandHeap(size);
		release_lock(heap_lock);
		
		return malloc(size);
	}

	void HeapSegmentHeader::mergeCurrentNextToLast(){
		if(this->nextSegment == tail){
			if(this->nextSegment->nextSegment){
				tail = this->nextSegment->nextSegment;
			}
			else {
				tail = this->lastSegment;
			}
		}

		if(this->nextSegment == head){
			if(this->nextSegment->lastSegment){
				head = this->nextSegment->lastSegment;
			}
			else{
				head = this->nextSegment->nextSegment;
			}
		}

		if(this == tail){
			if(this->nextSegment->nextSegment){
				tail = this->nextSegment->nextSegment;
			}
			else {
				tail = this->lastSegment;
			}
		}

		if(this == head){
			if(this->lastSegment){
				head = this->lastSegment;
			}
			else {
				tail = this->lastSegment;
			}
		}

		this->lastSegment->length += this->length + sizeof(struct HeapSegmentHeader) + this->nextSegment->length + sizeof(struct HeapSegmentHeader);
		this->lastSegment->nextSegment = this->nextSegment->nextSegment;
		this->nextSegment->nextSegment->lastSegment = this->lastSegment;

		memset(this->nextSegment, 0, sizeof(struct HeapSegmentHeader));
		memset(this, 0, sizeof(struct HeapSegmentHeader));
	}

	void HeapSegmentHeader::mergeCurrentToLast(){

		this->lastSegment->length += this->length + sizeof(struct HeapSegmentHeader);
		this->lastSegment->nextSegment = this->nextSegment;
		this->nextSegment->lastSegment = this->lastSegment;

		if(this == tail){
			if(this->nextSegment->nextSegment){
				tail = this->nextSegment;
			}
			else {
				tail = this->lastSegment;
			}
		}

		if(this == head){
			if(this->lastSegment){
				head = this->lastSegment;
			}
			else {
				head = this->nextSegment;
			}
		}

		memset(this, 0, sizeof(struct HeapSegmentHeader));
	}

	void HeapSegmentHeader::mergeNextToCurrent(){
		struct HeapSegmentHeader* nextHeader = this->nextSegment;

		this->length += this->nextSegment->length + sizeof(struct HeapSegmentHeader);
		this->nextSegment = this->nextSegment->nextSegment;
		this->nextSegment->lastSegment = this;

		if(this == tail){
			if(this->nextSegment->nextSegment){
				tail = this->nextSegment->nextSegment;
			}
			else {
				tail = this;
			}
		}

		if(nextHeader == tail){
			if(this->nextSegment){
				tail = this->nextSegment;
			}
			else{
				tail = this;
			}
		}

		if(this == head){
			head = this->lastSegment;
		}

		memset(nextHeader, 0, sizeof(struct HeapSegmentHeader));
	}

	void HeapSegmentHeader::split(size_t size){
		if(this->length < size + sizeof(struct HeapSegmentHeader)){
			return;
		}

		struct HeapSegmentHeader* newSegment = (struct HeapSegmentHeader*)((uint64_t)this + sizeof(struct HeapSegmentHeader) + size);
		memset(newSegment, 0, sizeof(struct HeapSegmentHeader));

		newSegment->isFree = true;
		newSegment->length = this->length - size - sizeof(struct HeapSegmentHeader);
		newSegment->nextSegment = this->nextSegment;
		newSegment->lastSegment = this;

		if(!this->nextSegment){
			tail = newSegment;
		}

		this->nextSegment = newSegment;
		this->length = size;
	}

	size_t getSize(void* ptr){
		struct HeapSegmentHeader* s = reinterpret_cast<struct HeapSegmentHeader*>(ptr) - 1;
		return s->length;
	}

	void* calloc(size_t num, size_t size){
		hasBeenInit();

		void *ptr = malloc(num * size);
		
		if(!ptr){
			return nullptr;
		}

		memset(ptr, 0, num * size);
		return ptr;
	}

	void *realloc(void* ptr, size_t size){
		hasBeenInit();

		if(!ptr){
			return malloc(size);
		}

		size_t oldSize = heap::getSize(ptr);

		if(!size){
			free(ptr);
			return nullptr;
		}

		if(size < oldSize){
			oldSize = size;
		}

		void* newPtr = heap::malloc(size);
		
		if(!newPtr){
			return ptr;
		}

		memcpy(newPtr, ptr, oldSize);
		free(ptr);
		return newPtr;
	}

	void expandHeap(size_t length){
		length += sizeof(struct HeapSegmentHeader);

		if(length % 0x1000){
			length -= length % 0x1000;
			length += 0x1000;
		}

		size_t pageCount = length / 0x1000;

		struct HeapSegmentHeader* newSegment = reinterpret_cast<struct HeapSegmentHeader*>(heapEnd);
		for(size_t i = 0; i < pageCount; ++i){
			// TODO null pointer exception
			vMemory::kernel_pagemap->mapMem((uint64_t)heapEnd, (uint64_t)pMemory::requestPage());
			heapEnd = reinterpret_cast<void*>(reinterpret_cast<size_t>(heapEnd) + 0x1000);
		}

		if(tail && tail->isFree){
			tail->length += length;
		}
		else {
			newSegment->length = length - sizeof(struct HeapSegmentHeader);
			newSegment->isFree = true;
			newSegment->lastSegment = tail;
			newSegment->nextSegment = nullptr;

			if(tail){
				tail->nextSegment = newSegment;
			}

			tail = newSegment;
		}

		if(!head){
			head = newSegment;
		}

		totalSize += length + sizeof(struct HeapSegmentHeader);
		freeSize -= length + sizeof(HeapSegmentHeader);

		//serial::log("[RAM] heap expand: %zu bytes", length);
	}
}

void* operator new(size_t size){
	return heap::malloc(size);
}

void *operator new[](size_t size){
	return heap::malloc(size);
}

void operator delete(void* ptr){
	heap::free(ptr);
}