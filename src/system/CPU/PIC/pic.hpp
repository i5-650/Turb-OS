#pragma once

#include <stdint.h>

namespace turbo::pic {
	enum PIC{
		PIC1 = 0x20,
		PIC2 = 0xA0,
		PIC1_COMMAND = PIC1,
		PIC1_DATA = (PIC1+1),
		PIC2_COMMAND = PIC2,
		PIC2_DATA = (PIC2+1),
		PIC_END_OF_INTERRUPT = 0x20
	};

	void endOfInterrupt(uint64_t interruptNumber);
	void disable();
	void init();
}