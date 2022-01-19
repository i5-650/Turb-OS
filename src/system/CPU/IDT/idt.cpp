#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <lib/lock.hpp>
#include <lib/portIO.hpp>
#include <system/CPU/PIC/pic.hpp>
#include <system/CPU/APIC/apic.hpp>

using namespace turbo;


namespace turbo::idt {

	DEFINE_LOCK(idt_lock);
	bool isInit = false;

	idtEntry_t idt[256];
	idtr_t idtr;

	intHandler_t interrupt_handlers[256];

	void idtSetDescriptor(uint8_t vector, void *isr, uint8_t type_attr){
		idt[vector].offset_1 = (uint64_t)isr & 0xFFFF;
		idt[vector].selector = 0x28;
		idt[vector].ist = 0;
		idt[vector].type_attr = type_attr;
		idt[vector].offset_2 = ((uint64_t)isr >> 16) & 0xFFFF;
		idt[vector].offset_3 = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
		idt[vector].zero = 0;
	}

	void reload(){
		asm volatile ("cli");
		asm volatile ("lidt %0" : : "memory"(idtr));
		asm volatile ("sti");
	}

	extern "C" void *int_table[];

	void init(){
		serial::log("[+] Initialising IDT");

		if(isInit){
			serial::log("[!!] Already init: IDT\n");
			return;
		}

		idt_lock.lock();

		idtr.limit = (uint16_t)sizeof(idtEntry_t) * 256 - 1;
		idtr.base = (uintptr_t)&idt[0];

		for(size_t i = 0; i < 256; i++){
			idtSetDescriptor(i, int_table[i], 0x8E);
		}
		reload();

		serial::newline();
		isInit = true;
		idt_lock.unlock();
	}

	void registerInterruptHandler(uint8_t vector, intHandler_t handler){
		interrupt_handlers[vector] = handler;
		if(apic::isInit && vector > 31 && vector < 48){
			apic::ioapicRedirectIRQ(vector - 32, vector);
		}
	}

	static const char *exception_messages[32] = {
		"Division by zero",
		"Debug",
		"Non-maskable interrupt",
		"Breakpoint",
		"Detected overflow",
		"Out-of-bounds",
		"Invalid opcode",
		"No coprocessor",
		"Double fault",
		"Coprocessor segment overrun",
		"Bad TSS",
		"Segment not present",
		"Stack fault",
		"General protection fault",
		"Page fault",
		"Unknown interrupt",
		"Coprocessor fault",
		"Alignment check",
		"Machine check",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
	};

	static volatile bool halt = true;
	void exception_handler(registers_t *regs){
		serial::log("System exception! %s", (char*)exception_messages[regs->int_no & 0xff]);
		serial::log("Error code: 0x%lX", regs->error_code);

		switch (regs->int_no){
		}

		printf("PANIC Exception: %s\n", (char*)exception_messages[regs->int_no & 0xff]);

		switch (regs->int_no){
			case 8:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
				printf("PANIC Error code: 0x%lX\n", regs->error_code);
				break;
		}

		printf("PANIC System halted!\n");
		serial::log("[/!\\]System halted\n");
		asm volatile ("cli; hlt");
	}

	void irq_handler(registers_t *regs){
		if(interrupt_handlers[regs->int_no]){
			interrupt_handlers[regs->int_no](regs);
		}

		if(apic::isInit){
			apic::endOfInterrupt();
		}
		else{
			pic::endOfInterrupt(regs->int_no);
		}
	}

	void int_handler(registers_t *regs){
		if (regs->int_no < 32){
			exception_handler(regs);
		}
		irq_handler(regs);
	}
}