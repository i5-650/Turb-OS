#pragma once
#include <system/CPU/GDT/gdt.hpp>
#include <stddef.h>
#include <system/CPU/scheduling/scheduler/scheduler.hpp>

using namespace turbo;



#define thisCPU \
({ \
	uint64_t cpuNumber; \
	asm volatile("movq %%gs:[0], %0" : "=r"(cpuNumber) : : "memory"); \
	&turbo::smp::cpus[cpuNumber]; \
})

namespace turbo::smp {

	struct cpu_t {
		uint64_t cpuID;
		uint32_t lapicID;
		gdt::TSS* tss;

		size_t fpuStorageSize;
		void (*fpuSave)(void*);
		void (*fpuRestore)(void*);

		scheduler::thread_t* currentThread;
		scheduler::process_t* currentProcess;
		scheduler::process_t* idleP;

		bool isUp;
	};

	extern cpu_t *cpus;
	extern bool isInit;

	void init();
}
