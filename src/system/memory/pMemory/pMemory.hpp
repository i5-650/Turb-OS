#pragma once

#include <system/memory/bitmap/bitmap.hpp>
#include <kernel/main.hpp>
#include <stdint.h>

namespace turbo::pMemory {
	extern Bitmap bitmap;
	extern bool isInit;

	void* alloc(size_t count = 1);
	void* realloc(void* ptr, size_t oldSize = 1, size_t newSize = 1);
	void free(void* ptr, size_t size = 1);

	size_t getFreeRam();
	size_t getUsedRam();

	void init();
}