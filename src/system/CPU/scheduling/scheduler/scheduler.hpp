#pragma once 

#include <lib/cpu/cpu.hpp>
#include <lib/lock.hpp>
#include <stdint.h>
#include <system/memory/heap/heap.hpp>
#include <lib/TurboVector/TurboVector.hpp>
#include <system/memory/vMemory/vMemory.hpp>

using namespace turbo;

namespace turbo::scheduler {
	#define DEFAULT_TIMESLICE 5

	enum state_t{
		INITIAL_STATE,
		READY,
		RUNNING,
		BLOCKED,
		SLEEPING,
		KILLED
	};

	struct process_t;

	struct thread_t{
		int TID = 1;
		state_t state;
		uint8_t *thread_stack;
		registers_t thread_regs;
		process_t *parent_proc;
		size_t sliceOfTime = DEFAULT_TIMESLICE;
	};

	struct process_t{
		char name[128];
		int PID = 0;
		int nextTID = 1;
		state_t state;
		vMemory::Pagemap *pagemap;
		TurboVector<thread_t*> threadsVec;
		TurboVector<process_t*> children;
		process_t *parent;
	};

	extern process_t *initproc;

	extern TurboVector<process_t*> proc_table;

	extern size_t proc_count;
	extern size_t thread_count;

	thread_t *allocThread(uint64_t addr, uint64_t args);
	thread_t *createThread(uint64_t addr, uint64_t args, process_t *parent = nullptr);

	process_t *allocProc(const char *name, uint64_t addr, uint64_t args);
	process_t *createProc(const char *name, uint64_t addr, uint64_t args);

	thread_t *this_thread();
	process_t *this_proc();

	void blockThread();
	void blockThread(thread_t *thread);

	void blockProc();
	void blockProc(process_t *proc);

	void unblockThread(thread_t *thread);
	void unblockProc(process_t *proc);

	void exitThread();
	void exitProc();

	static inline int getPID(){
		return this_proc()->PID;
	}

	static inline int getTID(){
		return this_thread()->TID;
	}

	void _yield(uint64_t ms = 1);
	void switchTask(registers_t *regs);

	void init();
	extern bool isInit;
}
