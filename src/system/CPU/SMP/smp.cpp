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
#include <system/CPU/scheduling/scheduler/scheduler.hpp>

using namespace turbo;

namespace turbo::smp {

	bool isInit = false;

	DEFINE_LOCK(cpu_lock);
	volatile int cpusUp = 0;
	cpu_t *cpus = nullptr;

	extern "C" void InitSSE();
	static void cpu_init(stivale2_smp_info *cpu)
	{
		cpu_lock.lock();
		gdt::reloadAll(cpu->lapic_id);
		idt::reload();

		vMemory::switchPagemap(vMemory::kernel_pagemap);

		set_kernel_gs(static_cast<uintptr_t>(cpu->extra_argument));
		set_user_gs(static_cast<uintptr_t>(cpu->extra_argument));

		thisCPU->lapicID = cpu->lapic_id;
		thisCPU->tss = &gdt::tss[thisCPU->lapicID];

		enableSSE();
		enableSMEP();
		enableSMAP();
		enableUMIP();

		uint32_t a = 0, b = 0, c = 0, d = 0;
		__get_cpuid(1, &a, &b, &c, &d);
		if ((c & bit_XSAVE))
		{
			write_cr(4, read_cr(4) | (1 << 18));
			
			uint64_t xcr0 = 0;
			xcr0 |= (1 << 0);
			xcr0 |= (1 << 1);
			if ((c & bit_AVX)) xcr0 |= (1 << 2);
			
			if (__get_cpuid(7, &a, &b, &c, &d))
			{
				if ((b & bit_AVX512F))
				{
					xcr0 |= (1 << 5);
					xcr0 |= (1 << 6);
					xcr0 |= (1 << 7);
				}
			}
			wrxcr(0, xcr0);
			
			thisCPU->fpuStorageSize = c;
			
			thisCPU->fpuSave = xsave;
			thisCPU->fpuRestore = xrstor;
		}
		else
		{
			thisCPU->fpuStorageSize = 512;
			thisCPU->fpuSave = fxsave;
			thisCPU->fpuRestore = fxrstor;
		}

		serial::log("CPU %ld is up", thisCPU->lapicID);
		thisCPU->isUp = true;
		cpusUp++;

		cpu_lock.unlock();
		if (cpu->lapic_id != smp_tag->bsp_lapic_id)
		{
			if (apic::isInit) apic::lapicInit(thisCPU->lapicID);
			scheduler::init();
		}
	}

	void init()
	{
		serial::log("Initialising SMP");

		if (isInit)
		{
			serial::log("CPUs are already up!\n");
			return;
		}

		cpus = static_cast<cpu_t*>(calloc(smp_tag->cpu_count, sizeof(cpu_t)));

		for (size_t i = 0; i < smp_tag->cpu_count; i++)
		{
			smp_tag->smp_info[i].extra_argument = (uint64_t)&cpus[i];
			cpus[i].cpuID = i;

			uint64_t sched_stack = reinterpret_cast<uint64_t>(malloc(STACK_SIZE));
			gdt::tss[i].IST[0] = sched_stack;

			if (smp_tag->bsp_lapic_id != smp_tag->smp_info[i].lapic_id)
			{
				uint64_t stack = reinterpret_cast<uint64_t>(malloc(STACK_SIZE));
				gdt::setStack(i, stack);

				smp_tag->smp_info[i].target_stack = stack + STACK_SIZE;
				smp_tag->smp_info[i].goto_address = reinterpret_cast<uintptr_t>(cpu_init);
			}
			else cpu_init(&smp_tag->smp_info[i]);
		}

		while (static_cast<uint64_t>(cpusUp) < smp_tag->cpu_count);

		serial::log("All CPUs are up\n");
		isInit = true;
	}
}