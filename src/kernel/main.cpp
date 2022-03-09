#pragma region include
#include <drivers/display/framebuffer/framebuffer.hpp>
#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/CPU/GDT/gdt.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <system/PCI/pci.hpp>
#include <kernel/kernel.hpp>
#include <lib/string.hpp>
#include <lib/memory/memory.hpp>
#include <lib/panic.hpp>
#include <lib/lock.hpp>
#include <stivale2.h>
#include <system/CPU/SMP/smp.hpp>
#include <drivers/devices/mouse.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/ACPI/acpi.hpp>
#include <apps/turboShell.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <system/CPU/scheduling/RTC/rtc.hpp>
#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <drivers/fs/vfs/turboVFS.hpp>
#include <system/CPU/scheduling/PIT/pit.hpp>
#include <drivers/display/ssfn/ssfn.hpp>
#pragma endregion include

namespace turbo {

	void myTime(){
		while(true){
			size_t size = 0;
			for(size_t i = 0; i < STACK_SIZE; ++i){
				if(kernelStack[i] != 'A'){
					break;
				}
				size++;
			}
			ssfn::setColor(ssfn::fgcolor, 0xFF0000); // red
			ssfn::printfAt(0,0,"%s", rtc::getTime());
			ssfn::printfAt(0, 1, "FREE RAM: %zu KB", (pMemory::getFreeRam() / 1024 / 1024));
		}
	}

	void main(){

		turbo::serial::log("Turb OS");

		turbo::serial::log("CPU cores available: %d", smp_tag->cpu_count);
		turbo::serial::log("Total usable memory: %ld MB\n", getmemsize() / 1024 / 1024);
		turbo::serial::log("Arguments passed to kernel: %s", cmdline);

		turbo::serial::log("Available kernel modules:");

		for(uint64_t t = 0; t < mod_tag->module_count; t++){
			turbo::serial::log("%d) %s", t + 1, mod_tag->modules[t].string);
		}

		turbo::terminal::center("Welcome Turb OS");

		printf("CPU cores available: %ld\n", smp_tag->cpu_count);
		printf("Total usable memory: %ld MB\n", getmemsize() / 1024 / 1024);

		turbo::terminal::check("Initialising PMM...");
		turbo::pMemory::init();
		turbo::terminal::okerr(pMemory::isInit);

		turbo::terminal::check("Initialising VMM...");
		turbo::vMemory::init();
		turbo::terminal::okerr(vMemory::isInit);

		turbo::terminal::check("Initialising Heap...");
		turbo::terminal::okerr(true);

		turbo::terminal::check("Initialising Global Descriptor Table...");
		turbo::gdt::init();
		turbo::terminal::okerr(gdt::isInit);

		turbo::terminal::check("Initialising Interrupt Descriptor Table...");
		turbo::idt::init();
		turbo::terminal::okerr(idt::isInit);

		turbo::terminal::check("Initialising ACPI...");
		turbo::acpi::init();
		turbo::terminal::okerr(acpi::isInit);

		turbo::terminal::check("Initialising HPET...");
		turbo::hpet::init();
		turbo::terminal::okerr(turbo::hpet::isInit);

		turbo::terminal::check("Initialising PIT...");
		turbo::pit::init();
		turbo::terminal::okerr(turbo::pit::isInit);

		turbo::terminal::check("Initialising PCI...");
		turbo::pci::init();
		turbo::terminal::okerr(pci::isInit);

		turbo::terminal::check("Initialising APIC...");
		turbo::apic::init();
		turbo::terminal::okerr(apic::isInit);

		turbo::terminal::check("Initialising SMP...");
		turbo::smp::init();
		turbo::terminal::okerr(turbo::smp::isInit);

		turbo::terminal::check("Initialising VFS ...");
		turbo::vfs::init();
		turbo::terminal::okerr(turbo::vfs::isInit);


		turbo::terminal::check("Initialising PS/2 Keyboard...");
		turbo::keyboard::init();
		turbo::terminal::okerr(turbo::keyboard::isInit);

		turbo::terminal::check("Initialising PS/2 Mouse...");
		turbo::mouse::init();
		turbo::terminal::okerr(turbo::mouse::isInit);

		//turbo::shell::run();
		scheduler::createProc("Init", (uint64_t)turbo::shell::run, 0);
		//turbo::serial::log("Starting shell");
		scheduler::createThread((uint64_t)myTime, 0, scheduler::initproc);

		//printf("good2\n");
		//turbo::shell::run();

		scheduler::init();
	}
}
