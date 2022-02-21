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
#pragma endregion include

namespace turbo {

	struct stivale2_struct_tag_smp *smp_tag;
	struct stivale2_struct_tag_memmap *mmap_tag;
	struct stivale2_struct_tag_rsdp *rsdp_tag;
	struct stivale2_struct_tag_framebuffer *frm_tag;
	struct stivale2_struct_tag_terminal *term_tag;
	struct stivale2_struct_tag_modules *mod_tag;
	struct stivale2_struct_tag_cmdline *cmd_tag;
	struct stivale2_struct_tag_kernel_file_v2 *kfilev2_tag;
	struct stivale2_struct_tag_hhdm *hhdm_tag;
	struct stivale2_struct_tag_pmrs *pmrs_tag;
	struct stivale2_struct_tag_kernel_base_address *kbaddr_tag;

	char *cmdline;

	int find_module(const char *name){
		for (uint64_t i = 0; i < mod_tag->module_count; i++){
			if (!strcmp(mod_tag->modules[i].string, name)){
				return i;
			}
		}
		return -1;
	}

	void myTime(){
		while(true){
			size_t size = 0;
			for(size_t i = 0; i < STACK_SIZE; ++i){
				if(kernelStack[i] != 'A'){
					break;
				}
				size++;
			}

			printf("clock\n");
		}
	}

	void main(struct stivale2_struct *stivale2_struct){
		smp_tag = (stivale2_struct_tag_smp (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_SMP_ID);
		mmap_tag = (stivale2_struct_tag_memmap (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);
		rsdp_tag = (stivale2_struct_tag_rsdp (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_RSDP_ID);
		frm_tag = (stivale2_struct_tag_framebuffer (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
		term_tag = (stivale2_struct_tag_terminal (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);
		mod_tag = (stivale2_struct_tag_modules (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MODULES_ID);
		cmd_tag = (stivale2_struct_tag_cmdline (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_CMDLINE_ID);
		kfilev2_tag = (stivale2_struct_tag_kernel_file_v2 (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_KERNEL_FILE_V2_ID);
		hhdm_tag = (stivale2_struct_tag_hhdm (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_HHDM_ID);
		pmrs_tag = (stivale2_struct_tag_pmrs (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_PMRS_ID);
		kbaddr_tag = (stivale2_struct_tag_kernel_base_address (*))stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_KERNEL_BASE_ADDRESS_ID);
		

		cmdline = (char *)cmd_tag->cmdline;

		if(!strstr(cmdline, "nocom")){
			turbo::serial::init();
		}

		turbo::serial::log("Turb OS");

		turbo::serial::log("CPU cores available: %d", smp_tag->cpu_count);
		turbo::serial::log("Total usable memory: %ld MB\n", getmemsize() / 1024 / 1024);
		turbo::serial::log("Arguments passed to kernel: %s", cmdline);

		turbo::serial::log("Available kernel modules:");

		for(uint64_t t = 0; t < mod_tag->module_count; t++){
			turbo::serial::log("%d) %s", t + 1, mod_tag->modules[t].string);
		}

		turbo::serial::newline();

		if(frm_tag == NULL){
			PANIC("Could not find framebuffer tag!");
		}

		turbo::framebuffer::init();

		if(term_tag == NULL){
			PANIC("Could not find terminal tag!");
		}

		turbo::terminal::init();

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
		turbo::heap::init();
		turbo::terminal::okerr(heap::isInit);

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

		rtc::init();
		printf("good\n");

		//turbo::shell::run();
		scheduler::createProcess("INIT", (uint64_t)turbo::shell::run, 0);
		scheduler::createThread((uint64_t)myTime, 0, scheduler::initProc);

		printf("good2\n");
		//turbo::shell::run();

		scheduler::init();
		printf("good3\n");

	}
}
