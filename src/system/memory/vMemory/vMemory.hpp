#pragma once

#include <lib/lock.hpp>
#include <stdint.h>
#include <stddef.h>

namespace turbo::vMemory {

	enum PT_Flag{
		present = (1 << 0),
		readNwrite = (1 << 1),
		userSuper = (1 << 2),
		writeThrough = (1 << 3),
		cacheDisable = (1 << 4),
		accessed = (1 << 5),
		largerPages = (1 << 7),
		custom0 = (1 << 9),
		custom1 = (1 << 10),
		custom2 = (1 << 11),
		NX = (1UL << 63)
	};

	struct PDEntry{
		uint64_t value = 0;

		void setFlag(PT_Flag flag, bool enabled);
		void setFlags(uint64_t flags, bool enabled);

		bool getFlag(PT_Flag flag);
		bool getFlags(uint64_t flags);

		void setAddress(uint64_t address);
		uint64_t getAddress();
	};

	struct [[gnu::aligned(0x1000)]] PTable{
		PDEntry entries[512];
	};

	struct Pagemap{
		lock_t lock;
		PTable *TOPLVL = nullptr;

		PDEntry &virt2pte(uint64_t vaddr);

		void mapMem(uint64_t vaddr, uint64_t paddr, uint64_t flags = (present | readNwrite));
		void remapMem(uint64_t vaddr_old, uint64_t vaddr_new, uint64_t flags = (present | readNwrite));
		void mapUserMem(uint64_t vaddr, uint64_t paddr, uint64_t flags = (present | readNwrite));
		void mapHHMem(uint64_t paddr, uint64_t flags = (present | readNwrite));
		void unmapMem(uint64_t vaddr);

		void setFlags(uint64_t vaddr, uint64_t flags);
		void remFlags(uint64_t vaddr, uint64_t flags);
	};

	extern bool isInit;
	extern bool lvl5;
	extern Pagemap *kernel_pagemap;

	Pagemap *newPagemap();
	Pagemap *clonePagemap(Pagemap *old);
	void switchPagemap(Pagemap *pmap);
	PTable *getPagemap();

	void init();
}