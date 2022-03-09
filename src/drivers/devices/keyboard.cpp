#include <drivers/devices/SCT.hpp>
#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <lib/string.hpp>
#include <lib/memory/memory.hpp>
#include <lib/lock.hpp>
#include <lib/portIO.hpp>
#include <system/memory/heap/heap.hpp>
#include <system/ACPI/acpi.hpp>
#include <lib/cpu/cpu.hpp>

namespace turbo::keyboard {
	bool isInit = false;

	char retstr[1024] = "\0";
	bool reading = false;
	int gi = 0;

	volatile bool pressed = false;
	volatile bool enter = false;

	kbd_mod_t kbd_mod;

	char getASCIIchar(uint8_t key_code){
		if(!kbd_mod.shift && !kbd_mod.capslock){
			return kbdus[key_code];
		}

		if(kbd_mod.shift && !kbd_mod.capslock){
			return kbdus_shft[key_code];
		}

		if(!kbd_mod.shift && kbd_mod.capslock){
			return kbdus_caps[key_code];
		}

		if(kbd_mod.shift && kbd_mod.capslock){
			return kbdus_capsshft[key_code];
		}

		return 0;
	}

	static void handleComb(uint8_t scancode){
		char ch = getASCIIchar(scancode);

		if(kbd_mod.ctrl && kbd_mod.alt && scancode == keys::DELETE){
			acpi::reboot();
		}
		else if(kbd_mod.ctrl && ((ch == 'l') || (ch == 'L'))){
			terminal::clear();
			if(reading){
				memset(retstr, '\0', 1024);
				enter = true;
			}
		}
	}

	char* buff;
	char c[10] = "\0";

	void clearBuffer(){
		for(size_t i = 0; i < strlen(buff); i++){
			buff[i] = '\0';
		}
	}

	static void Keyboard_Handler(registers_t *){
		uint8_t scancode = inb(0x60);

		if(scancode & 0x80){
			switch(scancode){
				// same case
				case keys::L_SHIFT_UP:
				case keys::R_SHIFT_UP:
					kbd_mod.shift = false;
					break;
				case keys::CTRL_UP:
					kbd_mod.ctrl = false;
					break;
				case keys::ALT_UP:
					kbd_mod.alt = false;
					break;
			}
		}
		else{
			switch(scancode){
				// same case
				case keys::L_SHIFT_DOWN:
				case keys::R_SHIFT_DOWN:
					kbd_mod.shift = true;
					break;
				case keys::CTRL_DOWN:
					kbd_mod.ctrl = true;
					break;
				case keys::ALT_DOWN:
					kbd_mod.alt = true;
					break;
				case keys::CAPSLOCK:
					kbd_mod.capslock = (!kbd_mod.capslock) ? true : false;
					break;
				case keys::NUMLOCK:
					kbd_mod.numlock = (!kbd_mod.numlock) ? true : false;
					break;
				case keys::SCROLLLOCK:
					kbd_mod.scrolllock = (!kbd_mod.scrolllock) ? true : false;
					break;
				case keys::RIGHT:
					strcpy(c, "\033[C");
					terminal::cursor_right();
					break;
				case keys::LEFT:
					strcpy(c, "\033[D");
					terminal::cursor_left();
					break;
				case keys::UP:
					strcpy(c, "\033[A");
					terminal::cursor_up();
					break;
				case keys::DOWN:
					strcpy(c, "\033[B");
					terminal::cursor_down();
					break;
				default:
					memset(c, 0, strlen(c));
					c[0] = getASCIIchar(scancode);
					if(kbd_mod.alt || kbd_mod.ctrl){
						handleComb(scancode);
					}
					else{
						switch(c[0]){
							case '\n':
								printf("\n");
								clearBuffer();
								enter = true;
								break;
							case '\b':
								if(buff[0] != '\0'){
									buff[strlen(buff) - 1] = '\0';
									if(reading){
										retstr[--gi] = 0;
									}
									printf("\b \b");
								}
								break;
							default:
								pressed = true;
								printf("%s", c);
								strcat(buff, c);
								break;
						}
					}
				break;
			}
		}
	}

	char getChar(){
		while(!pressed);
		pressed = false;
		return c[0];
	}

	DEFINE_LOCK(readLock);
	char *getLine(){
		readLock.lock();
		reading = true;
		memset(retstr, '\0', 1024);
		serial::log("enter %d", enter);
		serial::log("pressed %d", pressed);
		while(!enter){
			if(pressed){

				serial::log("if enter %d", enter);
				serial::log("if pressed %d", pressed);
				
				if(gi >= 1024 - 1){
					printf("\nBuffer Overflow !");
					enter = false;
					reading = false;
					gi = 0;
					readLock.unlock();
					return nullptr;
				}
				

				retstr[gi] = getChar();
				serial::log("%s", retstr);
				gi++;
			}
		}

		enter = false;
		reading = false;
		gi = 0;
		readLock.unlock();
		return retstr;
	}

	void init(){
		serial::log("[+] Initialising PS/2 Keyboard");

		if(isInit){
			serial::log("[!!] Already init: keyboard\n");
			return;
		}
		buff = (char*) calloc(1024, sizeof(char));
		idt::registerInterruptHandler(idt::IRQ1, Keyboard_Handler);
		serial::newline();
		isInit = true;
	}
}
