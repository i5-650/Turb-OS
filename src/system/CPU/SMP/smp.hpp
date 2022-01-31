#pragma once

#include <system/CPU/GDT/gdt.hpp>
#include <stddef.h>
#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <stdint.h>

using namespace turbo;

namespace turbo::smp {

	#define thisCPU \
	({ \
		uint64_t cpuNumber; \
		asm volatile("movq %%gs:[0], %0" : "=r"(cpuNumber) : : "memory"); \
		&turbo::smp::cpus[cpuNumber]; \
	})

	struct cpu_t {
		uint64_t cpuID;
		uint32_t lapicID;
		gdt::TSS* tss;

		size_t fpuStorageSize;
		void (*fpuSave)(void*);
		void (*fpuRestore)(void*);

		volatile bool isUp;
		scheduler::thread_t* currentThread;
		scheduler::process_t* currentProcess;
		scheduler::process_t* idleP;
	};

	extern cpu_t *cpus;
	extern bool isInit;

	void init();
}
