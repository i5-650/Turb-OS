#include <stdint.h>
#include <stddef.h>

namespace turbo::vMemory {

	enum PT_Flag{
		present = ((uint64_t)1 << 0),
		readWrite = ((uint64_t)1 << 1),
		userSuper = ((uint64_t)1 << 2),
		writeThrough = ((uint64_t)1 << 3),
		cacheDisable = ((uint64_t)1 << 4),
		accessed = ((uint64_t)1 << 5),
		largerPages = ((uint64_t)1 << 7),
		custom0 = ((uint64_t)1 << 9),
		custom1 = ((uint64_t)1 << 10),
		custom2 = ((uint64_t)1 << 11),
		NX = ((uint64_t)1 << 63)
	};

	struct PDEntry{
		uint64_t value;

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
		PTable *PML4;

		void mapMem(uint64_t vaddr, uint64_t paddr, uint64_t flags = (present | readWrite));
		void mapUserMem(uint64_t vaddr, uint64_t paddr, uint64_t flags = (present | readWrite));
		void unmapMem(uint64_t vaddr);
		void remapMem(uint64_t vaddressOld, uint64_t vaddressNew, uint64_t flags = (present | readWrite));

		void setFlags(uint64_t vaddress, uint64_t flags);
		void removeFlags(uint64_t vaddress, uint64_t flags);
	};

	extern bool isInit;
	extern Pagemap *kernel_pagemap;

	Pagemap *newPagemap();
	Pagemap *clonePagemap(Pagemap *old);
	void switchPagemap(Pagemap *pmap);

	void init();
}