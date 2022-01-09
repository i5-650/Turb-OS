#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <stddef.h>

[[noreturn]] void panic(const char *message, const char *file, size_t line){
	turbo::serial::log("%s", message);
	turbo::serial::log("File: %s", file);
	turbo::serial::log("Line: %zu", line);
	turbo::serial::log("System halted!\n");

	printf("\n[\033[31mPANIC\033[0m] %s", message);
	printf("\n[\033[31mPANIC\033[0m] File: %s", file);
	printf("\n[\033[31mPANIC\033[0m] Line: %zu", line);
	printf("\n[\033[31mPANIC\033[0m] System halted!\n");

	while(true){
		asm volatile ("cli; hlt");
	}
}