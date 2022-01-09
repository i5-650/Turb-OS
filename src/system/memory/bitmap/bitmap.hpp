#pragma once

#include <stddef.h>
#include <stdint.h>

class Bitmap{
	public:
	size_t size;
	uint8_t *buffer;
	bool operator[](uint64_t index);
	bool set(uint64_t index, bool value);
	bool get(uint64_t index);
};