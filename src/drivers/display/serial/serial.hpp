#pragma once

namespace turbo::serial {

	// https://wiki.osdev.org/Serial_Ports
	enum COMS{
		COM1 = 0x3F8,
		COM2 = 0x2F8
	};

	extern bool initialised;

	void printc(char c, void *arg);
	void serial_printf(const char *fmt, ...);
	void newline();

	void log(const char *fmt, ...);

	void init();
}