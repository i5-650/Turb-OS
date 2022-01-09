#include <drivers/display/framebuffer/framebuffer.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/memory/heap/heap.hpp>
#include <kernel/main.hpp>
#include <lib/string.hpp>
#include <lib/memory/memory.hpp>

using namespace turbo::heap;


namespace turbo::framebuffer {
	uint64_t frm_addr;
	uint16_t frm_width;
	uint16_t frm_height;
	// how many bytes to skip to go one pixel down
	uint16_t frm_pitch;
	uint16_t frm_bpp;
	uint16_t frm_pixperscanline;
	uint32_t frm_size;

	uint32_t cursorbuffer[16 * 19];
	uint32_t cursorbuffersecond[16 * 19];

	void putpix(uint32_t x, uint32_t y, uint32_t colour){
		*(uint32_t*)((uint64_t)frm_addr + (x * 4) + (y * frm_pixperscanline * 4)) = colour;
	}

	void putpix(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint64_t b){
		*(uint32_t*)((uint64_t)frm_addr + (x * 4) + (y * frm_pixperscanline * 4)) = (r << 16) | (g << 8) | b;
	}

	uint32_t getpix(uint32_t x, uint32_t y){
		return *(uint32_t*)((uint64_t)frm_addr + (x * 4) + (y * frm_pixperscanline * 4));
	}

	void framebuffer_restore(uint32_t *frm){
		memcpy((void*)frm_addr, frm, frm_height * frm_pitch);
		free(frm);
	}

	uint32_t *framebuffer_backup(){
		uint32_t *frm = (uint32_t*)malloc(frm_height * frm_pitch);
		memcpy(frm, (void*)frm_addr, frm_height * frm_pitch);
		return frm;
	}

	void init(){
		serial::log("Initialising framebuffer\n");

		frm_addr = frm_tag->framebuffer_addr;
		frm_width = frm_tag->framebuffer_width;
		frm_height = frm_tag->framebuffer_height;
		frm_pitch = frm_tag->framebuffer_pitch;
		frm_bpp = frm_tag->framebuffer_bpp;
		frm_pixperscanline = frm_pitch / 4;

		frm_size = frm_height * frm_pitch;
	}
}