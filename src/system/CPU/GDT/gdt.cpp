#include <drivers/display/serial/serial.hpp>
#include <system/memory/heap/heap.hpp>
#include <system/CPU/GDT/gdt.hpp>
#include <kernel/main.hpp>
#include <lib/memory/memory.hpp>
#include <lib/lock.hpp>

namespace turbo::gdt {

	[[gnu::aligned(0x1000)]]
	GDT DefaultGDT = {
		{0x0000, 0, 0, 0x00, 0x00, 0},
		{0xFFFF, 0, 0, 0x9A, 0x80, 0},
		{0xFFFF, 0, 0, 0x92, 0x80, 0},
		{0xFFFF, 0, 0, 0x9A, 0xCF, 0},
		{0xFFFF, 0, 0, 0x92, 0xCF, 0},
		{0x0000, 0, 0, 0x9A, 0xA2, 0},
		{0x0000, 0, 0, 0x92, 0xA0, 0},
		{0x0000, 0, 0, 0xF2, 0x00, 0},
		{0x0000, 0, 0, 0xFA, 0x20, 0},
		{0x0000, 0, 0, 0x89, 0x00, 0}
	};

	DEFINE_LOCK(gdt_lock)
	bool isInit = false;
	GDTDescriptor gdtDescriptor;
	TSS *tss;

	void reloadGDT(){
		loadGDT(&gdtDescriptor);
	}

	void reloadTSS(){
		loadTSS();
	}

	void reloadAll(int cpu){
		acquire_lock(gdt_lock);

		uintptr_t base = (uintptr_t)&tss[cpu];

		DefaultGDT.Tss.length = base + sizeof(tss[cpu]);
		DefaultGDT.Tss.base0 = base;
		DefaultGDT.Tss.base1 = base >> 16;
		DefaultGDT.Tss.flag1 = 0x89;
		DefaultGDT.Tss.flag2 = 0x00;
		DefaultGDT.Tss.base2 = base >> 24;
		DefaultGDT.Tss.base3 = base >> 32;
		DefaultGDT.Tss.Reserved = 0x00;

		reloadGDT();
		reloadTSS();

		release_lock(gdt_lock);
	}

	void init(){
		serial::log("[+] Initialising GDT");

		if (isInit){
			serial::log("[!!] already init: GDT\n");
			return;
		}

		tss = (TSS*)heap::calloc(smp_tag->cpu_count, sizeof(TSS));

		gdtDescriptor.size = sizeof(GDT) - 1;
		gdtDescriptor.offset = (uint64_t)&DefaultGDT;

		reloadAll(smp_tag->bsp_lapic_id);

		serial::newline();
		isInit = true;
	}

	void setStack(uint64_t cpu, uintptr_t stack){
		tss[cpu].RSP[0] = stack;
	}

	uint64_t getStack(uint64_t cpu){
		return tss[cpu].RSP[0];
	}
	/*
	void setStack(uintptr_t stack){
		tss[this_cpu->lapic_id].RSP[0] = stack;
	}

	uint64_t getStack(){
		return tss[this_cpu->lapic_id].RSP[0];
	}*/
}