#pragma once

#include <lib/math.hpp>
#include <stivale2.h>

namespace turbo::framebuffer {

	extern uint64_t frm_addr;
	extern uint16_t frm_width;
	extern uint16_t frm_height;
	extern uint16_t frm_pitch;
	extern uint16_t frm_bpp;
	extern uint16_t frm_pixperscanline;
	extern uint32_t frm_size;

	void putPixel(uint32_t x, uint32_t y, uint32_t colour);
	void putPixel(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint64_t b);
	uint32_t getPixel(uint32_t x, uint32_t y);

	void restoreFramebuffer(uint32_t *frm);
	uint32_t *framebufferBackup();

	void drawLine(int x0, int y0, int x1, int y1, uint32_t colour);

	void drawRectangle(int x, int y, int w, int h, uint32_t colour);
	void drawFilledRectangle(int x, int y, int w, int h, uint32_t colour);
	
	void drawCircle(int xm, int ym, int r, uint32_t colour);
	void drawFilledCircle(int cx, int cy, int radius, uint32_t colour);
	
	void drawOverCursor(uint8_t cursor[], struct point pos, uint32_t colour, bool back);
	void clearCursor(uint8_t cursor[], struct point pos);

	void init();
}