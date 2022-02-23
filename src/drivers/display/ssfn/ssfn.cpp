#include <drivers/display/framebuffer/framebuffer.hpp>
#include <drivers/display/terminal/terminal.hpp>
#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include <ssfn.h>


namespace turbo::ssfn{
	uint64_t bgcolor;
	uint64_t fgcolor = 0xFFFFFF;
	point pos;

	void printc(char c, void* args){
		ssfn_putc(c);
	}

	void setColor(uint64_t fg = fgcolor, uint64_t bg = bgcolor){
		bgcolor = bg;
		fgcolor = fg;
		ssfn_dst.fg = fgcolor;
		ssfn_dst.bg = bgcolor;
	}

	void setPosition(uint64_t x, uint64_t y){
		pos.X = (++x) * 8 - 8;
		pos.Y = (++y) * 16 - 16;
		ssfn_dst.x = pos.X;
		ssfn_dst.y = pos.Y;
	}

	void setpPosition(uint64_t x, uint64_t y){
		pos.X = x;
		pos.Y = y;

		ssfn_dst.x = x;
		ssfn_dst.y = y;
	}

	void printf(const char* fmt, ...){
		va_list args;
    	va_start(args, fmt);
    	vfctprintf(&printc, nullptr, fmt, args);
    	va_end(args);

    	pos.X = ssfn_dst.x / 8;
    	pos.Y = ssfn_dst.y / 16;
	}
	void printfAt(uint64_t x, uint64_t y, const char* fmt, ...){
    	x++;
    	y++;
    	ssfn_dst.x = x * 8 - 8;
    	ssfn_dst.y = y * 16 - 16;

    	va_list args;
    	va_start(args, fmt);
    	vfctprintf(&printc, nullptr, fmt, args);
    	va_end(args);

    	pos.X = ssfn_dst.x / 8;
    	pos.Y = ssfn_dst.y / 16;
	}

	void ppintf(const char* fmt, ...){
    	va_list args;
    	va_start(args, fmt);
    	vfctprintf(&printc, nullptr, fmt, args);
    	va_end(args);

    	pos.X = ssfn_dst.x;
    	pos.Y = ssfn_dst.y;
	}
	void pprintfAt(uint64_t x, uint64_t y, const char* fmt, ...){
	    ssfn_dst.x = x;
    	ssfn_dst.y = y;

    	va_list args;
    	va_start(args, fmt);
    	vfctprintf(&printc, nullptr, fmt, args);
    	va_end(args);

    	pos.X = ssfn_dst.x;
    	pos.Y = ssfn_dst.y;
	}

	extern "C" uint64_t _binary_font_sfn_start;
	void init(){
		ssfn_src = reinterpret_cast<ssfn_font_t*>(&_binary_font_sfn_start);

    	ssfn_dst.ptr = reinterpret_cast<uint8_t*>(framebuffer::frm_addr);
    	ssfn_dst.w = framebuffer::frm_width;
    	ssfn_dst.h = framebuffer::frm_height;
    	ssfn_dst.p = framebuffer::frm_pitch;
    	ssfn_dst.x = ssfn_dst.y = 0;
    	pos.X = 0;
    	pos.Y = 0;
    	ssfn_dst.fg = fgcolor;
    	ssfn_dst.bg = bgcolor;
	}
} 
