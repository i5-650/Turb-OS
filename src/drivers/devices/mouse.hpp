#pragma once

#include <lib/math.hpp>
#include <stdint.h>

namespace turbo::mouse {

	#define PS2_X_SIGN 0b00010000
	#define PS2_Y_SIGN 0b00100000
	#define PS2_X_OVER 0b01000000
	#define PS2_Y_OVER 0b10000000

	enum MouseState{
		PS2_NONE = 0b00000000,
		PS2_LEFT = 0b00000001,
		PS2_MIDDLE = 0b00000100,
		PS2_RIGHT = 0b00000010,
	};

	extern bool isInit;

	extern point position;
	extern point oldPosition;
	extern uint32_t mouseBorderCol;
	extern uint32_t mouseInsideCol;

	MouseState getMouseState();

	void draw();
	void clear();

	void init();
}