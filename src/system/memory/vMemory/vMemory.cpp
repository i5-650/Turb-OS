#include <drivers/display/serial/serial.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <kernel/main.hpp>
#include <lib/memory/memory.hpp>
#include <lib/math.hpp>
#include <lib/lock.hpp>
#include <lib/cpu/cpu.hpp>

namespace turbo::vMemory {

	bool isInit = false;
	Pagemap *kernel_pagemap = nullptr;
	DEFINE_LOCK(VMemoryLock);

	PTable* getNextLevel(PTable* currentLevel, size_t entry){
		PTable* ret;

		if(currentLevel->entries[entry].getFlag(present)){
			ret = (struct PTable*)((uint64_t)currentLevel->entries[entry].getAddress() << 12);
		}
		else {
			ret = (PTable*)turbo::pMemory::requestPage();
			memset(ret, 0, 4096);
			currentLevel->entries[entry].setAddress((uint64_t) ret >> 12);
			currentLevel->entries[entry].setFlags(present | readWrite, true);
		}

		return ret;
	}

	void Pagemap::setFlags(uint64_t vaddress, uint64_t flags){
		size_t pml4_entry = (vaddress & ((uint64_t)0x1FF << 39)) >> 39;
		size_t pml3_entry = (vaddress & ((uint64_t)0x1FF << 30)) >> 30;
		size_t pml2_entry = (vaddress & ((uint64_t)0x1FF << 21)) >> 21;
		size_t pml1_entry = (vaddress & ((uint64_t)0x1FF << 12)) >> 12;

		PTable *pml4 = this->PML4, *pml3, *pml2, *pml1;

		pml3 = getNextLevel(pml4, pml4_entry);
		pml2 = getNextLevel(pml3, pml3_entry);
		pml1 = getNextLevel(pml2, pml2_entry);

		pml1->entries[pml1_entry].setFlags(flags, true);
	}

	void Pagemap::removeFlags(uint64_t vaddress, uint64_t flags){
		size_t pml4_entry = (vaddress & ((uint64_t)0x1FF << 39)) >> 39;
		size_t pml3_entry = (vaddress & ((uint64_t)0x1FF << 30)) >> 30;
		size_t pml2_entry = (vaddress & ((uint64_t)0x1FF << 21)) >> 21;
		size_t pml1_entry = (vaddress & ((uint64_t)0x1FF << 12)) >> 12;

		PTable *pml4 = this->PML4, *pml3, *pml2, *pml1;

		pml3 = getNextLevel(pml4, pml4_entry);
		pml2 = getNextLevel(pml3, pml3_entry);
		pml1 = getNextLevel(pml2, pml2_entry);

		pml1->entries[pml1_entry].setFlags(flags, false);	
	}

	void Pagemap::mapMem(uint64_t vaddress, uint64_t paddress, uint64_t flags){
		VMemoryLock.lock();
		size_t pml4_entry = (vaddress & ((uint64_t)0x1FF << 39)) >> 39;
		size_t pml3_entry = (vaddress & ((uint64_t)0x1FF << 30)) >> 30;
		size_t pml2_entry = (vaddress & ((uint64_t)0x1FF << 21)) >> 21;
		size_t pml1_entry = (vaddress & ((uint64_t)0x1FF << 12)) >> 12;

		PTable *pml4 = this->PML4, *pml3, *pml2, *pml1;

		pml3 = getNextLevel(pml4, pml4_entry);
		pml2 = getNextLevel(pml3, pml3_entry);
		pml1 = getNextLevel(pml2, pml2_entry);

		pml1->entries[pml1_entry].setAddress(paddress >> 12);
		pml1->entries[pml1_entry].setFlags(flags, true);

		VMemoryLock.unlock();
	}
	void Pagemap::remapMem(uint64_t vaddressOld, uint64_t vaddressNew, uint64_t flags){
		VMemoryLock.lock();
		uint64_t paddress = 0;

		size_t pml4_entry = (vaddressOld & ((uint64_t)0x1FF << 39)) >> 39;
		size_t pml3_entry = (vaddressOld & ((uint64_t)0x1FF << 30)) >> 30;
		size_t pml2_entry = (vaddressOld & ((uint64_t)0x1FF << 21)) >> 21;
		size_t pml1_entry = (vaddressOld & ((uint64_t)0x1FF << 12)) >> 12;

		PTable *pml4 = this->PML4, *pml3, *pml2, *pml1;

		pml3 = getNextLevel(pml4, pml4_entry);
		pml2 = getNextLevel(pml3, pml3_entry);
		pml1 = getNextLevel(pml2, pml2_entry);

		paddress = pml1->entries[pml1_entry].getAddress() << 12;
		pml1->entries[pml1_entry].value = 0;
		asm volatile ("invlpg (%0)" :: "r"(vaddressOld));

		VMemoryLock.unlock();
		this->mapMem(vaddressNew, paddress, flags);
	}

	void Pagemap::unmapMem(uint64_t vaddress){
		VMemoryLock.lock();

		size_t pml4_entry = (vaddress & ((uint64_t)0x1FF << 39)) >> 39;
		size_t pml3_entry = (vaddress & ((uint64_t)0x1FF << 30)) >> 30;
		size_t pml2_entry = (vaddress & ((uint64_t)0x1FF << 21)) >> 21;
		size_t pml1_entry = (vaddress & ((uint64_t)0x1FF << 12)) >> 12;

		PTable *pml4 = this->PML4, *pml3, *pml2, *pml1;

		pml3 = getNextLevel(pml4, pml4_entry);
		pml2 = getNextLevel(pml3, pml3_entry);
		pml1 = getNextLevel(pml2, pml2_entry);

		pml1->entries[pml1_entry].value = 0;
		asm volatile ("invlpg (%0)" :: "r"(vaddress));
		VMemoryLock.unlock();
	}

	void Pagemap::mapUserMem(uint64_t vaddress, uint64_t paddress, uint64_t flags){
		mapMem(vaddress, paddress, flags | userSuper);
	}

	void PDEntry::setFlag(PT_Flag flag, bool isEnable){
		uint64_t bitSel = (uint64_t)flag;
		value &= ~bitSel;
		if(isEnable){
			value |= bitSel;
		}
	}

	void PDEntry::setFlags(uint64_t flags, bool isEnable){
		uint64_t bitSel = flags;
		value &= ~bitSel;
		if (isEnable){
			value |= bitSel;
		}
	}

	bool PDEntry::getFlag(PT_Flag flag){
		uint64_t bitSel = (uint64_t)flag;
		return (value & (bitSel > 0)) ? true : false;
	}

	bool PDEntry::getFlags(uint64_t flags){
		return (value & (flags > 0)) ? true : false;
	}

	uint64_t PDEntry::getAddress(){
		return (value & 0x000FFFFFFFFFF000) >> 12;
	}

	void PDEntry::setAddress(uint64_t address){
		address &= 0x000000FFFFFFFFFF;
		value &= 0xFFF0000000000FFF;
		value |= (address << 12);
	}

	Pagemap *newPagemap(){
		Pagemap *pagemap = new Pagemap;

		if(kernel_pagemap == nullptr){
			pagemap->PML4 = (PTable*)turbo::pMemory::requestPage();
			return pagemap;
		}
		
		PTable *pml4 = pagemap->PML4;
		PTable *kernel_pml4 = kernel_pagemap->PML4;

		for(size_t i = 0; i < 512;i++){
			pml4->entries[i] = kernel_pml4->entries[i];
		}

		return pagemap;
	}

	Pagemap *clonePagemap(Pagemap *old){
		Pagemap *pagemap = new Pagemap;
		pagemap->PML4 = (PTable*)turbo::pMemory::requestPage();

		PTable *pml4 = pagemap->PML4;
		PTable *old_pml4 = old->PML4;

		for(size_t i = 0; i < 512; i++){
			pml4->entries[i] = old_pml4->entries[i];
		}

		return pagemap;
	}

	void switchPagemap(Pagemap *pmap){
		// TODO  null pointer exception
		write_cr(3, reinterpret_cast<uint64_t>(pmap->PML4));
	}

	PTable* getPagemap(){
		return ((PTable*) read_cr(3));
	}

	void init(){
		turbo::serial::log("[+] Initialising virtual memory\n");

		if(isInit){
			turbo::serial::log("[!!] Already init: virtual memory\n");
			return;
		}
		// TODO null pointer exception
		kernel_pagemap->PML4 = (PTable*)read_cr(3);
		//turbo::serial::log("so far so good");
		switchPagemap(kernel_pagemap);
		//turbo::serial::log("so fat so bigg");
		turbo::serial::newline();
		isInit = true;
	}
}