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
		int tid = 1;
		state_t state;
		uint8_t *stack;
		registers_t regs;
		process_t *parent;
		size_t sliceOfTime = DEFAULT_TIMESLICE;
	};

	struct process_t{
		char name[128];
		int pid = 0;
		int next_tid = 1;
		state_t state;
		vMemory::Pagemap *pagemap;
		TurboVector<thread_t*> threads;
		TurboVector<process_t*> children;
		process_t *parent;
	};

	extern process_t *initproc;

	extern TurboVector<process_t*> proc_table;

	extern size_t proc_count;
	extern size_t thread_count;

	thread_t *thread_alloc(uint64_t addr, uint64_t args);
	thread_t *thread_create(uint64_t addr, uint64_t args, process_t *parent = nullptr);

	process_t *proc_alloc(const char *name, uint64_t addr, uint64_t args);
	process_t *proc_create(const char *name, uint64_t addr, uint64_t args);

	thread_t *this_thread();
	process_t *this_proc();

	void thread_block();
	void thread_block(thread_t *thread);

	void proc_block();
	void proc_block(process_t *proc);

	void thread_unblock(thread_t *thread);
	void proc_unblock(process_t *proc);

	void thread_exit();
	void proc_exit();

	static inline int getpid(){
		return this_proc()->pid;
	}

	static inline int gettid(){
		return this_thread()->tid;
	}

	void _yield(uint64_t ms = 1);
	void switchTask(registers_t *regs);

	void init();
	extern bool isInit;
}
