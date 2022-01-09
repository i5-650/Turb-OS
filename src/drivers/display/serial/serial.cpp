/**
 * Serial port = communications port on common IBM PC
 * Now it's more USB and modern periphical interfaces
 * But it's easier to use than USB (i'm lazy) common on x86 sys
 * 
 */

#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <lib/lock.hpp>
#include <lib/portIO.hpp>

namespace turbo::serial {

	bool isInit = false;
	DEFINE_LOCK(lock)

	bool check(){
		if(isInit){
			return true;
		}
		else{
			return false;
		}
	}

	int is_transmit_empty(){
		return inb(COMS::COM1 + 5) & 0x20;
	}

	int received(){
		return inb(COMS::COM1 + 5) & 1;
	}

	char read(){
		while(!received());
		return inb(COMS::COM1);
	}

	void printc(char c, [[gnu::unused]] void *arg){
		if (!check()){
			return;
		}

		while(!is_transmit_empty());
		outb(COMS::COM1, c);
	}

	// to receive an unknown number of args
	void serialPrintf(const char *fmt, ...){
		acquire_lock(lock);
		va_list args;
		va_start(args, fmt);
		vfctprintf(&printc, nullptr, fmt, args);
		va_end(args);
		release_lock(lock);
	}
	
	void log(const char *fmt, ...){
		acquire_lock(lock);
		va_list args;
		va_start(args, fmt);
		vfctprintf(&printc, nullptr, "[LOG] ", args);
		vfctprintf(&printc, nullptr, fmt, args);
		vfctprintf(&printc, nullptr, "\n", args);
		va_end(args);
		release_lock(lock);
	}

	void newline(){
		serialPrintf("\n");
	}

	static void COM1_Handler(registers_t *){
		char c = read();
		switch(c){
			case 13:
				c = '\n';
				break;
			case 8:
			case 127:
				printf("\b ");
				serialPrintf("\b ");
				c = '\b';
				break;
		}
		printf("%c", c);
		serialPrintf("%c", c);
	}

	void init(){
		if(isInit){
			return;
		}

		/**
		 * +1 = activacte interrupts
		 * +0 = reading in buffer and write other buffer
		 * +2 = id interrupts
		 * +3 = line control
		 * +4 = modem control
		 * 
		 */
		outb(COMS::COM1 + 1, 0x00);
		outb(COMS::COM1 + 3, 0x80);
		outb(COMS::COM1 + 0, 0x03);
		outb(COMS::COM1 + 1, 0x00);
		outb(COMS::COM1 + 3, 0x03);
		outb(COMS::COM1 + 2, 0xC7);
		outb(COMS::COM1 + 4, 0x0B);

		// white
		serialPrintf("\033[0m");

		registerInterruptHandler(idt::IRQ4, COM1_Handler);
		outb(COMS::COM1 + 1, 0x01);

		isInit = true;
	}
}