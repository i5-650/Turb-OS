#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <kernel/main.hpp>
#include <lib/string.hpp>
#include <lib/lock.hpp>
#include <stivale2.h>
#include <kernel/kernel.hpp>

namespace turbo::terminal {

	DEFINE_LOCK(termLock);

	uint16_t columns;
	uint16_t rows;

	// white
	char *colour = (char*)"\033[0m";

	void (*write)(const char *string, uint64_t length);

	void init(){
		serial::log("[+] Initialising terminal\n");

		//void *write_ptr = (void*)term_tag->term_write;
		write = reinterpret_cast<void (*)(const char *, uint64_t)>(term_tag->term_write);
		columns = term_tag->cols;
		rows = term_tag->rows;
	}

	#pragma region Print
	void print(const char *string){
		termLock.lock();
		write(string, strlen(string));
		termLock.unlock();
	}

	// to print intergers
	void printi(int num){
		if(num != 0){
			char temp[10];
			int i = 0;

			if(num < 0){
				printc('-');
				num = -num;
			}

			if(num <= 0){
				// temp[1] = '8';
				// temp[0] = '9';
				// and so on 
				temp[i++] = '8';
				num = -(num / 10);
			}

			while(num > 0){
				temp[i++] = num % 10 + '0';
				num /= 10;
			}

			while (--i >= 0){
				printc(temp[i]);
			}
		}
		else{
			printc('0');
		}
	}

	void printc(char c){
		print(char2str(c));
	}
	#pragma endregion Print

	#pragma region Color
	void setcolour(const char *ascii_colour){
		colour = (char*)ascii_colour;
		printf("%s", colour);
	}

	void resetcolour(){
		// white
		colour = (char*)"\033[0m";
		printf("%s", colour);
	}
	#pragma endregion Color

	#pragma region Clear
	void reset(){
		termLock.lock();
		write("", STIVALE2_TERM_FULL_REFRESH);
		termLock.unlock();
	}

	void clear(const char *ansii_colour){
		turbo::keyboard::clearBuffer();
		setcolour(ansii_colour);
		// this clear the terminal
		printf("\033[H\033[2J");
		reset();
	}
	#pragma endregion Clear

	#pragma region CursorCtrl
	void cursor_up(int lines){
		printf("\033[%dA", lines);
	}

	void cursor_down(int lines){
		printf("\033[%dB", lines);
	}

	void cursor_right(int lines){
		printf("\033[%dC", lines);
	}

	void cursor_left(int lines){
		printf("\033[%dD", lines);
	}
	#pragma endregion CursorCtrl

	#pragma region Misc
	void center(const char *text){
		for(uint64_t i = 0; i < columns / 2 - strlen(text) / 2; i++){
			printc(' ');
		}
		print(text);
		for(uint64_t i = 0; i < columns / 2 - strlen(text) / 2; i++){
			printc(' ');
		}
	}

	void check(const char *message){
		// [*] %s underlined
		printf("%s ", message);
	}

	void okerr(bool ok){
		if(ok){
			printf("OK\n");
		}
		else{
			printf("XX\n");
		}
	}
	#pragma endregion Misc

	}

void _putchar(char character){
	turbo::terminal::printc(character);
}