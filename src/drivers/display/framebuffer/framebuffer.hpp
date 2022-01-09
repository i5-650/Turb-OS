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

	void putpix(uint32_t x, uint32_t y, uint32_t colour);
	void putpix(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint64_t b);
	uint32_t getpix(uint32_t x, uint32_t y);

	void framebuffer_restore(uint32_t *frm);
	uint32_t *framebuffer_backup();

	void init();
}