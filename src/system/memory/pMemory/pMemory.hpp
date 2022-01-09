#pragma once

#include <system/memory/bitmap/bitmap.hpp>
#include <kernel/main.hpp>
#include <stdint.h>

namespace turbo::pMemory {

	extern Bitmap pageBitmap;
	extern bool isInit;

	void freePage(void *address);
	void lockPage(void *address);
	void freePages(void *address, uint64_t pageCount);
	void lockPages(void *address, uint64_t pageCount);

	void *requestPage();
	void *requestPages(uint64_t count);

	uint64_t getFreeRam();
	uint64_t getUsedRam();
	uint64_t getReservedRam();

	void Bitmap_init(size_t bitmapSize, uintptr_t bufferAddr);
	void reservePage(void *address);
	void unreservePage(void *address);
	void reservePages(void *address, uint64_t pageCount);
	void unreservePages(void *address, uint64_t pageCount);

	void init();
}