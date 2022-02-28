#pragma once

#include <stdint.h>
#include <kernel/kernel.hpp>

namespace turbo::keyboard {
	
	struct kbd_mod_t {
		int shift : 1;
		int ctrl : 1;
		int alt : 1;
		int numlock : 1;
		int capslock : 1;
		int scrolllock : 1;
	};

	extern bool isInit;
	extern char* buff;

	void clearBuffer();

	char getChar();
	char *getLine();

	void init();
}