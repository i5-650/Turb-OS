#include <drivers/display/terminal/terminal.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/CPU/SMP/smp.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/CPU/PIC/pic.hpp>
#include <lib/panic.hpp>
#include <lib/lock.hpp>
#include <drivers/display/serial/serial.hpp>
#include <lib/portIO.hpp>

namespace turbo::idt {

	DEFINE_LOCK(idt_lock);
	bool isInit = false;

	IDTEntry idt[256];
	IDTPtr idtr;

	intHandler_t interrupt_handlers[256];

	void idtSetDescriptor(uint8_t vector, void *isr, uint8_t typeattr, uint8_t ist){
		idt[vector].Offset1 = reinterpret_cast<uint64_t>(isr);
		idt[vector].Selector = 0x28;
		idt[vector].IST = ist;
		idt[vector].TypeAttr = typeattr;
		idt[vector].Offset2 = reinterpret_cast<uint64_t>(isr) >> 16;
		idt[vector].Offset3 = reinterpret_cast<uint64_t>(isr) >> 32;
		idt[vector].Zero = 0;
	}

	void reload(){
		asm volatile ("cli");
		asm volatile ("lidt %0" : : "memory"(idtr));
		asm volatile ("sti");
	}

	void init(){
		serial::log("Initialising IDT");

		if (isInit){
			serial::log("IDT has already been initialised!\n");
			return;
		}

		idt_lock.lock();


		idtr.Limit = sizeof(IDTEntry) * 256 - 1;
		idtr.Base = (uintptr_t)&idt[0];

		for(size_t i = 0; i < 256; i++){
			idtSetDescriptor(i, int_table[i]);
		}
		idtSetDescriptor(SYSCALL, int_table[SYSCALL], 0xEE);

		pic::init();

		reload();

		serial::newline();
		isInit = true;
		idt_lock.unlock();
	}

	static uint8_t next_free = 48;
	uint8_t allocVector(){
		return (++next_free == SYSCALL ? ++next_free : next_free);
	}

	void registerInterruptHandler(uint8_t vector, intHandler_t handler, bool ioapic){
		interrupt_handlers[vector] = handler;
		if(ioapic && apic::isInit && vector > 31 && vector < 48){
			apic::ioapicRedirectIRQ(vector - 32,vector);
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
	static void exception_handler(registers_t *regs){
		serial::log("System exception!");
		serial::log("Exception: %s on CPU %d", (char*)exception_messages[regs->int_no], thisCPU->lapicID);
		serial::log("Error code: 0x%lX", regs->error_code);

		switch (regs->int_no)
		{
		}

		if(!halt){
			serial::newline();
			return;
		}

		printf("\n[\033[31mPANIC\033[0m] System Exception!\n");
		printf("[\033[31mPANIC\033[0m] Exception: %s on CPU %d\n", (char*)exception_messages[regs->int_no], thisCPU->lapicID);

		switch (regs->int_no){
			case 8:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
				printf("[\033[31mPANIC\033[0m] Error code: 0x%lX\n", regs->error_code);
				break;
		}

		printf("[\033[31mPANIC\033[0m] System halted!\n");
		serial::log("System halted!\n");
		if (scheduler::this_thread()->state == scheduler::RUNNING){
			asm volatile ("cli");
			thisCPU->currentThread->state = scheduler::READY;
			asm volatile ("sti");
		}
		asm volatile ("cli; hlt");
	}

	static void irq_handler(registers_t *regs){
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

	extern "C" void int_handler(registers_t *regs){
		if(regs->int_no < 32){
			exception_handler(regs);
		}
		else if(regs->int_no >= 32 && regs->int_no < 256){
			irq_handler(regs);
		}
		else{
			PANIC("Unknown interrupt!");
		}
	}
}