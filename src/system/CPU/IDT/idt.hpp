#pragma once

#include <stdint.h>
#include <lib/cpu/cpu.hpp>

namespace turbo::idt {

	#define SYSCALL 0x69

	enum IRQS{
		IRQ0 = 32,
		IRQ1 = 33,
		IRQ2 = 34,
		IRQ3 = 35,
		IRQ4 = 36,
		IRQ5 = 37,
		IRQ6 = 38,
		IRQ7 = 39,
		IRQ8 = 40,
		IRQ9 = 41,
		IRQ10 = 42,
		IRQ11 = 43,
		IRQ12 = 44,
		IRQ13 = 45,
		IRQ14 = 46,
		IRQ15 = 47,
	};

	struct [[gnu::packed]] idtEntry_t{
		uint16_t offset_1;
		uint16_t selector;
		uint8_t ist;
		uint8_t type_attr;
		uint16_t offset_2;
		uint32_t offset_3;
		uint32_t zero;
	};

	struct [[gnu::packed]] idtr_t{
		uint16_t limit;
		uint64_t base;
	};
	
	using intHandler_t = void (*)(registers_t *);

	extern idtEntry_t idt[];
	extern idtr_t idtr;

	extern intHandler_t interrupt_handlers[];

	extern bool isInit;

	void reload();

	uint8_t allocVector();
	void init();
	void registerInterruptHandler(uint8_t vector, intHandler_t handler);
	void idtSetDescriptor(uint8_t vector, void *isr, uint8_t type_attr = 0x8E, uint8_t ist = 0);

	extern "C" void int_handler(registers_t *regs);
	extern "C" void* int_table[];
}