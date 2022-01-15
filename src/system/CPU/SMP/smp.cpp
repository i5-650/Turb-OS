#include <system/CPU/APIC/apic.hpp>
#include <cpuid.h>
#include <drivers/display/serial/serial.hpp>
#include <lib/cpu/cpu.hpp>
#include <lib/lock.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <system/memory/heap/heap.hpp>
#include <kernel/kernel.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/CPU/SMP/smp.hpp>

namespace turbo::smp {
	bool isInit = false;

	DEFINE_LOCK(lockCPUSMP)

	volatile int cpusUp = 0;
	cpu_t* cpus;

	extern "C" void InitSSE();

	static void cpuInit(stivale2_smp_info* cpu){
		acquire_lock(lockCPUSMP);
		turbo::gdt::reloadAll(cpu->lapic_id);
		turbo::idt::reload();
		turbo:vMemory::switchPagemap(turbo::vMemory::kernel_pagemap);

		set_kernel_gs((uintptr_t)cpu->extra_argument);
		set_user_gs((uintptr_t)cpu->extra_argument);

		thisCPU->lapicID = cpu->lapic_id;
		thisCPU->tss = &turbo::gdt::tss[thisCPU->lapicID];

		enableSSE();
    	enableSMEP();
    	enableSMAP();
    	enableUMIP();

		uint64_t cr4 = 0;
		uint32_t a = 0, b = 0, c = 0, d = 0;
		__get_cpuid(1, &a, &b, &c, &d);
		if((c & bit_XSAVE)){
			cr4 = read_cr(4);
			cr4 |= (1 << 18);
			write_cr(4, cr4);

			uint64_t xcr0 = 0;
			xcr0 |= (1 << 0);
			xcr0 |= (1 << 1);

			if((c & bit_AVX)){
				xcr0 |= (1 << 2);
			} 

			if(__get_cpuid(7, &a, &b, &c, &d)){
				if((b & bit_AVX512F)){
					xcr0 |= (1 << 5);
					xcr0 |= (1 << 6);
					xcr0 |= (1 << 7);
				}
			}
			wrxcr(0, xcr0);

			thisCPU->fpuStorageSize = (size_t)c;
			thisCPU->fpuSave = xsave;
			thisCPU->fpuRestore = xrstor;
		}
		else {
			thisCPU->fpuStorageSize = 512;
			thisCPU->fpuSave = fxsave;
			thisCPU->fpuRestore = fxrstor;
		}

		turbo::serial::log("[**] SMP: %ld up\n", thisCPU->lapicID);
		thisCPU->isUp = true;
		cpusUp++;
		turbo::serial::log("%ld", cpusUp);

		release_lock(lockCPUSMP);

		if(cpu->lapic_id != smp_tag->bsp_lapic_id){
			if(turbo::apic::isInit){
				turbo::apic::lapicInit(thisCPU->lapicID);
			}
			while(true){
				asm volatile ("hlt");
			}
		}
	}

	void init(){
		turbo::serial::log("Initialising SMP\n");

		if(isInit){
			turbo::serial::log("already init: SMP\n");
			return;
		}

		cpus = (cpu_t*)turbo::heap::calloc(smp_tag->cpu_count, sizeof(cpu_t));

		for(size_t i = 0; i < smp_tag->cpu_count; i++){
			smp_tag->smp_info[i].extra_argument = (uint64_t)&cpus[i];

			uint64_t stack = (uint64_t)turbo::pMemory::requestPage();
			uint64_t schedulerStack = (uint64_t)turbo::pMemory::requestPage();

			turbo::gdt::tss[i].RSP[0] = stack;
			turbo::gdt::tss[i].IST[1] = schedulerStack;

			cpus[i].cpuID = i;

			if(smp_tag->bsp_lapic_id != smp_tag->smp_info[i].lapic_id){
				smp_tag->smp_info[i].target_stack = stack;
				smp_tag->smp_info[i].goto_address = (uintptr_t)cpuInit;
			}
			else {
				cpuInit(&smp_tag->smp_info[i]);
			}
		}

		while(cpusUp < smp_tag->cpu_count);

		turbo::serial::log("All cores up\n");

		isInit = true;
	}
}