#include <drivers/display/framebuffer/framebuffer.hpp>
#include <drivers/display/terminal/terminal.hpp>
#include <drivers/devices/mouse.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <lib/portIO.hpp>
#include <lib/cpu/cpu.hpp>
#include <kernel/kernel.hpp>

using namespace turbo;

namespace turbo::mouse {

	#define RED 0xff0000
	#define PURPLE 0xdd56f5
	#define BLUE 0x0000ff
	#define GREEN 0x00ff00
	

	bool isInit = false;

	uint8_t cycle = 0;
	uint8_t packet[4];
	bool packetReady = false;
	struct point position;
	struct point oldPosition;

	uint32_t mouseBorderCol = 0xFFFFFF;
	uint32_t mouseInsideCol = 0x2D2D2D;

	uint8_t cursorborder[]{
		0b10000000, 0b00000000,
		0b11000000, 0b00000000,
		0b10100000, 0b00000000,
		0b10010000, 0b00000000,
		0b10001000, 0b00000000,
		0b10000100, 0b00000000,
		0b10000010, 0b00000000,
		0b10000001, 0b00000000,
		0b10000000, 0b10000000,
		0b10000000, 0b01000000,
		0b10000000, 0b00100000,
		0b10000000, 0b00010000,
		0b10000001, 0b11110000,
		0b10001001, 0b00000000,
		0b10010100, 0b10000000,
		0b10100100, 0b10000000,
		0b11000010, 0b01000000,
		0b00000010, 0b01000000,
		0b00000001, 0b10000000,
	};

	uint8_t cursorinside[]{
		0b10000000, 0b00000000,
		0b11000000, 0b00000000,
		0b11100000, 0b00000000,
		0b11110000, 0b00000000,
		0b11111000, 0b00000000,
		0b11111100, 0b00000000,
		0b11111110, 0b00000000,
		0b11111111, 0b00000000,
		0b11111111, 0b10000000,
		0b11111111, 0b11000000,
		0b11111111, 0b11100000,
		0b11111111, 0b11110000,
		0b11111111, 0b11110000,
		0b11111111, 0b00000000,
		0b11110111, 0b10000000,
		0b11100111, 0b10000000,
		0b11000011, 0b11000000,
		0b00000011, 0b11000000,
		0b00000001, 0b10000000,
	};

	void mouseWait(){
		uint64_t timeout = 100000;
		while(timeout--){
			if((inb(0x64) & 0b10) == 0){
				return;
			}
		}
	}

	void mouseWaitInput(){
		uint64_t timeout = 100000;
		while(timeout--){
			if(inb(0x64) & 0b1){
				return;
			}
		}
	}

	void mouseWrite(uint8_t value){
		mouseWait();
		outb(0x64, 0xD4);
		mouseWait();
		outb(0x60, value);
	}

	uint8_t mouseRead(){
		mouseWaitInput();
		return inb(0x60);
	}

	MouseState getMouseState(){
		if(packet[0] & PS2_LEFT){
			return PS2_LEFT;
		}
		else if(packet[0] & PS2_MIDDLE){
			return PS2_MIDDLE;
		}
		else if(packet[0] & PS2_RIGHT){
			return PS2_RIGHT;
		}

		return PS2_NONE;
	}

	void draw(){
		framebuffer::drawOverCursor(cursorinside, position, mouseInsideCol, true);
		framebuffer::drawOverCursor(cursorborder, position, mouseBorderCol, false);
	}

	void clear(){
		framebuffer::clearCursor(cursorinside, oldPosition);
	}

	void proccessPacket(){
		if(!packetReady){
			return;
		}

		bool xmin, ymin, xover, yover;

		if(packet[0] & PS2_X_SIGN){
			xmin = true;
		}
		else{
			xmin = false;
		}

		if(packet[0] & PS2_Y_SIGN){
			ymin = true;
		}
		else{
			ymin = false;
		}

		if(packet[0] & PS2_X_OVER){
			xover = true;
		}
		else{
			xover = false;
		}

		if(packet[0] & PS2_Y_OVER){
			yover = true;
		}
		else{
			yover = false;
		}

		if(!xmin){
			position.X += packet[1];
			
			if(xover){
				position.X += 255;
			}
		}
		else{
			packet[1] = 256 - packet[1];
			position.X -= packet[1];
			
			if(xover){
				position.X -= 255;
			}
		}

		if(!ymin){
			position.Y -= packet[2];
			
			if(yover){
				position.Y -= 255;
			}
		}
		else{
			packet[2] = 256 - packet[2];
			position.Y += packet[2];
			if(yover){
				position.Y += 255;
			}
		}

		if(position.X < 0){
			position.X = 0;
		}

		if(position.X > framebuffer::frm_width - 1){
			position.X = framebuffer::frm_width - 1;
		}

		if(position.Y < 0){
			position.Y = 0;
		}

		if(position.Y > framebuffer::frm_height - 1){
			position.Y = framebuffer::frm_height - 1;
		}

		clear();

		static bool circle = false;
		switch(mouse::getMouseState()){
			case mouse::PS2_LEFT:
				if(circle){
					framebuffer::drawFilledCircle(mouse::position.X, mouse::position.Y, 5, 0x1fe079);  
				} 
				else{
					framebuffer::drawFilledRectangle(mouse::position.X, mouse::position.Y, 10, 10, 0x1fe079);
				}

				break;

			case mouse::PS2_MIDDLE:
				if(circle){
					circle = false;
				}
				else{
					circle = true;
				}

				break;

			case mouse::PS2_RIGHT:
				if(circle){
					framebuffer::drawFilledCircle(mouse::position.X, mouse::position.Y, 5, 0xff00dd);
				}   
				else{
					framebuffer::drawFilledRectangle(mouse::position.X, mouse::position.Y, 10, 10, 0xff00dd);
				}
				break;

			default:
				break;
		}

		draw();

		packetReady = false;
		oldPosition = position;
	}

	static void Mouse_Handler(registers_t *){
		uint8_t mousedata = inb(0x60);

		proccessPacket();

		static bool skip = true;
		if(skip){
			skip = false;
			return;
		}

		switch (cycle){
			case 0:
				if((mousedata & 0b00001000) == 0){
					break;
				}
				packet[0] = mousedata;
				cycle++;
				break;

			case 1:
				packet[1] = mousedata;
				cycle++;
				break;

			case 2:
				packet[2] = mousedata;
				packetReady = true;
				cycle = 0;
				break;
		}
	}

	void init(){
		serial::log("Initialising PS/2 mouse");

		if(isInit){
			serial::log("already init: mouse\n");
			return;
		}

		asm volatile ("cli");

		outb(0x64, 0xA8);

		mouseWait();
		outb(0x64, 0x20);
		mouseWaitInput();

		uint8_t status = inb(0x60);
		status |= 0b10;
		mouseWait();

		outb(0x64, 0x60);
		mouseWait();
		outb(0x60, status);

		mouseWrite(0xF6);
		mouseRead();
		mouseWrite(0xF4);
		mouseRead();

		asm volatile ("sti");

		registerInterruptHandler(idt::IRQ12, Mouse_Handler);

		serial::newline();
		isInit = true;
	}
}