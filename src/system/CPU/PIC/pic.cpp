#include <system/CPU/PIC/pic.hpp>
#include <lib/portIO.hpp>

namespace turbo::pic {

	void endOfInterrupt(uint64_t interruptNumber){
		if(interruptNumber >= 40){
			outb(pic::PIC2_COMMAND, pic::PIC_END_OF_INTERRUPT);
		}

		outb(pic::PIC1_COMMAND, PIC_END_OF_INTERRUPT);
	}

	void disable(){
		outb(0xA1, 0xFF);
		outb(0x21, 0xFF);
	}

	void init(){
		outb(0x20, 0x11);
		outb(0xA0, 0x11);
		outb(0x21, 0x20);
		outb(0xA1, 0x28);
		outb(0x21, 0x04);
		outb(0xA1, 0x02);
		outb(0x21, 0x01);
		outb(0xA1, 0x01);
		outb(0x21, 0x00);
		outb(0xA1, 0x00);
	}
}