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

using namespace turbo::heap;

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

	void handleComb(uint8_t scancode){
		char ch = getASCIIchar(scancode);

		if(kbd_mod.ctrl && kbd_mod.alt && scancode == keys::DELETE){
			acpi::reboot();
			terminal::print("Supposed to reboot");
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
		for(size_t i = 0; i < strlen(buff); ++i){
			buff[i] = '\0';
		}
	}

	static void Keyboard_Handler(turbo::idt::registers_t *){
		uint8_t scancode = inb(0x60);

		if(scancode & 0x80){
			switch(scancode){
				// same case
				case keys::L_SHIFT_UP:
				case keys::R_SHIFT_UP:
					kbd_mod.shift = 0;
					break;

				case keys::CTRL_UP:
					kbd_mod.ctrl = 0;
					break;

				case keys::ALT_UP:
					kbd_mod.alt = 0;
					break;
			}
		}
		else{
			switch(scancode){
				// same case
				case keys::L_SHIFT_DOWN:
				case keys::R_SHIFT_DOWN:
					kbd_mod.shift = 1;
					break;

				case keys::CTRL_DOWN:
					kbd_mod.ctrl = 1;
					break;

				case keys::ALT_DOWN:
					kbd_mod.alt = 1;
					break;

				case keys::CAPSLOCK:
					kbd_mod.capslock = (!kbd_mod.capslock) ? 1 : 0;
					break;

				case keys::NUMLOCK:
					kbd_mod.numlock = (!kbd_mod.numlock) ? 1 : 0;
					break;

				case keys::SCROLLLOCK:
					kbd_mod.scrolllock = (!kbd_mod.scrolllock) ? 1 : 0;
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
					strcpy(c, "\033[C");
					terminal::cursor_up();
					break;

				case keys::DOWN:
					strcpy(c, "\033[D");
					terminal::cursor_down();
					break;

				default:
					memset(c, 0, strlen(c));
					c[0] = getASCIIchar(scancode);
					if(kbd_mod.alt || kbd_mod.ctrl){
						char ch = char2up(c[0]);

						if(kbd_mod.ctrl){
							if(ch >= 'A' && ch <= '_' || ch == '?' || ch == '0'){
								printf("%c", escapes[char2num(ch)]);
							}
						}
						else if(kbd_mod.alt){
							printf("\x1b[%c", ch);
						}
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

	DEFINE_LOCK(getline_lock)
	char *getLine(){
		acquire_lock(getline_lock);
		reading = true;
		memset(retstr, '\0', 1024);
		while(!enter){
			if(pressed){
				if(gi >= 1024 - 1){
					printf("\nBuffer Overflow !");
					enter = false;
					reading = false;
					gi = 0;
					release_lock(getline_lock);
					return nullptr;
				}

				retstr[gi] = getChar();
				gi++;
			}
		}

		enter = false;
		reading = false;
		gi = 0;
		release_lock(getline_lock);
		return retstr;
	}

	void init(){
		serial::log("[+] Initialising PS/2 Keyboard");

		if(isInit){
			serial::log("[!!] Already init: keyboard\n");
			return;
		}
		buff = (char*)malloc(sizeof(char)* 10);
		buff[0] = '\0';
		registerInterruptHandler(idt::IRQ1, Keyboard_Handler);

		serial::newline();
		isInit = true;
	}
}