#include <drivers/display/framebuffer/framebuffer.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/memory/heap/heap.hpp>
#include <kernel/main.hpp>
#include <lib/string.hpp>
#include <lib/memory/memory.hpp>
#include <kernel/kernel.hpp>



namespace turbo::framebuffer {
	uint64_t frm_addr;
	uint16_t frm_width;
	uint16_t frm_height;
	// how many bytes to skip to go one pixel down
	uint16_t frm_pitch;
	uint16_t frm_bpp;
	uint16_t frm_pixperscanline;
	uint32_t frm_size;

	bool mousedrawn;

	uint32_t cursorbuffer[16 * 19];
	uint32_t cursorbuffersecond[16 * 19];

	void putPixel(uint32_t x, uint32_t y, uint32_t colour){
		*(uint32_t*)((uint64_t)frm_addr + (x * 4) + (y * frm_pixperscanline * 4)) = colour;
	}

	void putPixel(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint64_t b){
		*(uint32_t*)((uint64_t)frm_addr + (x * 4) + (y * frm_pixperscanline * 4)) = (r << 16) | (g << 8) | b;
	}

	uint32_t getPixel(uint32_t x, uint32_t y){
		return *(uint32_t*)((uint64_t)frm_addr + (x * 4) + (y * frm_pixperscanline * 4));
	}

	void restoreFramebuffer(uint32_t *frm){
		memcpy((void*)frm_addr, frm, frm_height * frm_pitch);
		free(frm);
	}

	uint32_t *framebufferBackup(){
		uint32_t *frm = (uint32_t*)malloc(frm_height * frm_pitch);
		memcpy(frm, (void*)frm_addr, frm_height * frm_pitch);
		return frm;
	}


	void drawVertLine(int x, int y, int dy, uint32_t colour){
		for (int i = 0; i < dy; i++){
			putPixel(x, y + i, colour);
		}
	}

	void drawHorLine(int x, int y, int dx, uint32_t colour){
		for (int i = 0; i < dx; i++){
			putPixel(x + i, y, colour);
		}
	}

	void drawDiagLine(int x0, int y0, int x1, int y1, uint32_t colour){
		int i;
		int sdx = sign(x1);
		int sdy = sign(y1);
		int dxabs = abs(x1);
		int dyabs = abs(y1);
		int x = dyabs >> 1;
		int y = dxabs >> 1;
		int px = x0;
		int py = y0;

		if(dxabs >= dyabs){
			for(i = 0; i < dxabs; i++){
				y += dyabs;
				
				if(y >= dxabs){
					y -= dxabs;
					py += sdy;
				}

				px += sdx;
				putPixel(px, py, colour);
			}
		}
		else{
			for(i = 0; i < dyabs; i++){
				x += dxabs;
				
				if(x >= dyabs){
					x -= dyabs;
					px += sdx;
				}

				py += sdy;
				putPixel(px, py, colour);
			}
		}
	}

	void drawLine(int x0, int y0, int x1, int y1, uint32_t colour){
		if(x0 > frm_width){
			x0 = frm_width - 1;
		}

		if(x1 > frm_width){
			x1 = frm_width - 1;
		}
		
		if(y0 > frm_height){
			y0 = frm_height - 1;
		}
		
		if(y1 > frm_height){
			y1 = frm_height - 1;
		}

		if(x0 < -1){
			x0 = -1;
		}

		if(x1 < -1){
			x1 = -1;
		}

		if(y0 < -1){
			y0 = -1;
		}

		if(y1 < -1){
			y1 = -1;
		}


		int dx = x1 - x0, dy = y1 - y0;

		if(dy == 0){
			drawHorLine(x0, y0, dx, colour);
			return;
		}
		
		if(dx == 0){
		
			drawVertLine(x0, y0, dy, colour);
		}
		drawDiagLine(x0, y0, dx, dy, colour);
	}

	void drawRectangle(int x, int y, int w, int h, uint32_t colour){
		drawLine(x, y, x + w, y, colour); // ¯
		drawLine(x, y + h - 1, x + w, y + h - 1, colour); // _
		drawLine(x, y, x, y + h, colour); // |*
		drawLine(x + w - 1, y, x + w - 1, y + h, colour); // *|
	}

	void drawFilledRectangle(int x, int y, int w, int h, uint32_t colour){
		for (int i = 0; i < h; i++){
			drawLine(x, y + i, x + w, y + i, colour);
		}
	}

	void drawCircle(int cx, int cy, int radius, uint32_t colour){
		int x = -radius, y = 0, err = 2 - 2 * radius;
		do{
			putPixel(abs(cx - x), abs(cy + y), colour);
			putPixel(abs(cx - y), abs(cy - x), colour);
			putPixel(abs(cx + x), abs(cy - y), colour);
			putPixel(abs(cx + y), abs(cy + x), colour);
			radius = err;

			if(radius > x){
				err += ++x * 2 + 1;
			}

			if(radius <= y){
				err += ++y * 2 + 1;
			}

		}while (x < 0);
	}

	void drawFilledCircle(int cx, int cy, int radius, uint32_t colour){
		if((radius > cx) | (radius > cy)){ 
			cx = radius; cy = radius;
		}

		int x = radius;
		int y = 0;
		int xC = 1 - (radius << 1);
		int yC = 0;
		int err = 0;

		while(x >= y){
			for(int i = cx - x; i <= cx + x; i++){
				putPixel(i, cy + y, colour);
				putPixel(i, cy - y, colour);
			}

			for(int i = cx - y; i <= cx + y; i++){
				putPixel(i, cy + x, colour);
				putPixel(i, cy - x, colour);
			}

			y++;
			err += yC;
			yC += 2;

			if(((err << 1) + xC) > 0){
				x--;
				err += xC;
				xC += 2;
			}
		}
	}

	void clearCursor(uint8_t cursor[], point pos){
		if(!mousedrawn){
			return;
		}

		int xmax = 16, ymax = 19, dx = frm_width - pos.X, dy = frm_height - pos.Y;

		if(dx < 16){
			xmax = dx;
		}

		if(dy < 19){
			ymax = dy;
		}


		for (int y = 0; y < ymax; y++){
			for (int x = 0; x < xmax; x++){
				int bit = y * 16 + x;
				int byte = bit / 8;
				if((cursor[byte] & (0b10000000 >> (x % 8)))){
					putPixel(pos.X + x, pos.Y + y, cursorbuffer[y * 16 + x]);
				}
			}
		}
	}

	void drawOverCursor(uint8_t cursor[], point pos, uint32_t colour, bool back){
		int xmax = 16, ymax = 19, dx = frm_width - pos.X, dy = frm_height - pos.Y;

		if(dx < 16){
			xmax = dx;
		}

		if(dy < 19){
			ymax = dy;
		}


		for (int y = 0; y < ymax; y++){
			for (int x = 0; x < xmax; x++){
				int bit = y * 16 + x;
				int byte = bit / 8;

				if((cursor[byte] & (0b10000000 >> (x % 8)))){
					if(back){
						cursorbuffer[y * 16 + x] = getPixel(pos.X + x, pos.Y + y);
					}

					putPixel(pos.X + x, pos.Y + y, colour);

					if(back){
						cursorbuffersecond[y * 16 + x] = getPixel(pos.X + x, pos.Y + y);
					}
				}
			}
		}

		mousedrawn = true;
	}

	void init(){
		turbo::serial::log("Initialising framebuffer\n");

		frm_addr = frm_tag->framebuffer_addr;
		frm_width = frm_tag->framebuffer_width;
		frm_height = frm_tag->framebuffer_height;
		frm_pitch = frm_tag->framebuffer_pitch;
		frm_bpp = frm_tag->framebuffer_bpp;
		frm_pixperscanline = frm_pitch / 4;

		frm_size = frm_height * frm_pitch;
	}
}