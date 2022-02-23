#pragma once 

#include <stdint.h>
#include <lib/math.hpp>

namespace turbo::ssfn{
	extern uint64_t bgcolor;
	extern uint64_t fgcolor;
	extern point pos;

	void setColor(uint64_t fg = fgcolor, uint64_t bg = bgcolor);

	void setPosition(uint64_t x, uint64_t y);
	void setpPosition(uint64_t x, uint64_t y);

	void printf(const char* fmt, ...);
	void printfAt(uint64_t x, uint64_t y, const char* fmt, ...);

	void ppintf(const char* fmt, ...);
	void pprintfAt(uint64_t x, uint64_t y, const char* fmt, ...);

	void init();
} 
