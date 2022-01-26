#pragma once
#include <stdint.h>
#include <stddef.h>

namespace turbo::gdt {

	struct [[gnu::packed]] GDTDescriptor{
		uint16_t size;
		uint64_t offset;
	};

	struct [[gnu::packed]] GDTEntry{
		uint16_t limit0;
		uint16_t base0;
		uint8_t base1;
		uint8_t accessByte;
		uint8_t granularity;
		uint8_t base2;
	};

	struct [[gnu::packed]] TSSEntry {
		uint16_t length;
		uint16_t base0;
		uint8_t base1;
		uint8_t flag1;
		uint8_t flag2;
		uint8_t base2;
		uint32_t base3;
		uint32_t Reserved;
	};

	// Global Descriptor Table
	struct [[gnu::packed, gnu::aligned(0x1000)]] GDT{
		struct GDTEntry Null;
		struct GDTEntry _16BitCode;
		struct GDTEntry _16BitData;
		struct GDTEntry _32BitCode;
		struct GDTEntry _32BitData;
		struct GDTEntry _64BitCode;
		struct GDTEntry _64BitData;
		struct GDTEntry UserData;
		struct GDTEntry UserCode;
		struct TSSEntry Tss;
	};

	// task statement segment
	struct [[gnu::packed]] TSS{
		uint32_t Reserved0;
		uint64_t RSP[3];
		uint64_t Reserved1;
		uint64_t IST[7];
		uint64_t Reserved2;
		uint16_t Reserved3;
		uint16_t IOPBOffset;
	};

	extern GDT DefaultGDT;
	extern bool isInit;
	extern TSS *tss;

	extern "C" void loadGDT(GDTDescriptor *gdtDescriptor);
	extern "C" void loadTSS();

	void reloadAll(int cpu);
	void reloadGDT();
	void reloadTSS();
	void init();

	void setStack(uint64_t cpu, uintptr_t stack);
	uint64_t getStack(uint64_t cpu);

	void setStack(uintptr_t stack);
	uint64_t getStack();
}