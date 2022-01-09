#include <drivers/display/serial/serial.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <kernel/main.hpp>
#include <lib/memory/memory.hpp>
#include <stivale2.h>
#include <lib/lock.hpp>
#include <lib/math.hpp>

namespace turbo::pMemory {

	Bitmap pageBitmap;
	bool isInit = false;

	uint64_t freeRAM;
	uint64_t reservedRAM;
	uint64_t usedRAM;

	uintptr_t highestPage = 0;

	extern "C" uint64_t __kernelstart;
	extern "C" uint64_t __kernelend;


	void init(){
		serial::log("[+] Initialising physical Memory");

		if (isInit){
			serial::log("[!!] Already init: physical Memory\n");
			return;
		}

		for (size_t i = 0; i < mmap_tag->entries; i++){
			if (mmap_tag->memmap[i].type != STIVALE2_MMAP_USABLE){
				continue;
			}

			uintptr_t top = mmap_tag->memmap[i].base;

			if (top > highestPage){
				highestPage = top;
			}
		}

		uint64_t memsize = getmemsize();
		freeRAM = memsize;
		uint64_t bitmapSize = memsize / 4096 / 8 + 1;

		Bitmap_init(bitmapSize, highestPage);

		reservePages(0, memsize / 4096 + 1);
		for (size_t i = 0; i < mmap_tag->entries; i++){
			if (mmap_tag->memmap[i].type != STIVALE2_MMAP_USABLE){
				continue;
			}
			unreservePages((void*)mmap_tag->memmap[i].base, mmap_tag->memmap[i].length / 4096);
		}
		reservePages(0, 0x100);
		lockPages(pageBitmap.buffer, pageBitmap.size / 4096 + 1);

		uint64_t kernelSize = (uint64_t)&__kernelend - (uint64_t)&__kernelstart;
		uint64_t kernelPageCount = (uint64_t)kernelSize / 4096 + 1;

		pMemory::lockPages((void*)&__kernelstart, kernelPageCount);

		serial::newline();
		isInit = true;
	}

	void Bitmap_init(size_t bitmapSize, uintptr_t bufferAddress){
		pageBitmap.size = bitmapSize;
		pageBitmap.buffer = (uint8_t*)bufferAddress;

		for (size_t i = 0; i < bitmapSize; i++){
			*(uint8_t*)(pageBitmap.buffer + i) = 0;
		}
	}

	uint64_t pageBitmapIndex = 0;
	void *requestPage(){
		for (; pageBitmapIndex < pageBitmap.size * 8; pageBitmapIndex++){
			if (pageBitmap[pageBitmapIndex] == true){
				continue;
			}
			lockPage((void*)(pageBitmapIndex * 4096));
			return (void*)(pageBitmapIndex * 4096);
		}
		return NULL;
	}

	// TODO change this awful code
	void *requestPages(uint64_t count){
		while (pageBitmapIndex < pageBitmap.size * 8){
			for (uint64_t i = 0; i < count; i++){
				if (pageBitmap[pageBitmapIndex + i] == true){
					pageBitmapIndex += i + 1;
					goto notfree;
				}
			}
			goto exit;
			notfree:
				continue;
			
			exit: {
				void* page = (void*)(pageBitmapIndex * 4096);
				pageBitmapIndex += count;
				lockPages(page, count);
				return page;
			}
		}
		return nullptr;
	}

	void freePage(void *address){
		uint64_t index = (uint64_t)address / 4096;
		
		if (pageBitmap[index] == false){
			return;
		}

		if (pageBitmap.set(index, false)){
			freeRAM += 4096;
			usedRAM -= 4096;
			
			if (pageBitmapIndex > index){
				pageBitmapIndex = index;
			}
		}
	}

	void freePages(void *address, uint64_t pageCount){
		for (size_t i = 0; i < pageCount; i++){
			freePage((void*)((uint64_t)address + (i * 4096)));
		}
	}

	void lockPage(void *address){
		uint64_t index = (uint64_t)address / 4096;

		if (pageBitmap[index] == true){
			return;
		}

		if (pageBitmap.set(index, true)){
			freeRAM -= 4096;
			usedRAM += 4096;
		}
	}

	void lockPages(void *address, uint64_t pageCount){
		for (uint64_t i = 0; i < pageCount; i++){
			lockPage((void*)((uint64_t)address + (i * 4096)));
		}
	}

	void unreservePage(void *address){
		uint64_t index = (uint64_t)address / 4096;
		if (pageBitmap[index] == false){
			return;
		}

		if (pageBitmap.set(index, false)){
			freeRAM += 4096;
			reservedRAM-= 4096;

			if (pageBitmapIndex > index){
				pageBitmapIndex = index;
			}
		}
	}

	void unreservePages(void *address, uint64_t pageCount){
		for (uint64_t i = 0; i < pageCount; i++){
			unreservePage((void*)((uint64_t)address + (i * 4096)));
		}
	}

	void reservePage(void *address){
		uint64_t index = (uint64_t)address / 4096;
		if (pageBitmap[index] == true){
			return;
		}

		if (pageBitmap.set(index, true)){
			freeRAM -= 4096;
			reservedRAM += 4096;
		}
	}

	void reservePages(void *address, uint64_t pageCount){
		for (uint64_t i = 0; i < pageCount; i++){
			reservePage((void*)((uint64_t)address + (i * 4096)));
		}
	}

	uint64_t getFreeRam(){
		return freeRAM;
	}

	uint64_t getUsedRam(){
		return usedRAM;
	}

	uint64_t getReservedRam(){
		return reservedRAM;
	}
}