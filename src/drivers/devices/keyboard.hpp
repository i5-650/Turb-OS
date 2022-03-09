#pragma once

#include <stdint.h>
#include <kernel/kernel.hpp>

namespace turbo::keyboard {
	
	struct kbd_mod_t {
		bool shift : 1;
		bool ctrl : 1;
		bool alt : 1;
		bool numlock : 1;
		bool capslock : 1;
		bool scrolllock : 1;
	};

	extern bool isInit;
	extern char* buff;

	void clearBuffer();

	char getChar();
	char *getLine();

	void init();
}