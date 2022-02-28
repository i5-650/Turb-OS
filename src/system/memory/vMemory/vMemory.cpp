#include <drivers/display/serial/serial.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <kernel/main.hpp>
#include <lib/memory/memory.hpp>
#include <lib/math.hpp>
#include <lib/lock.hpp>
#include <lib/cpu/cpu.hpp>
#include <kernel/kernel.hpp>

namespace turbo::vMemory {

	bool isInit = false;
	bool lvl5 = 0;
	Pagemap *kernel_pagemap = nullptr;

	PTable *get_next_lvl(PTable *curr_lvl, size_t entry){
		PTable *ret = nullptr;
		if (curr_lvl->entries[entry].getFlag(present)){
			ret = reinterpret_cast<PTable*>(static_cast<uint64_t>(curr_lvl->entries[entry].getAddress()) << 12);
		}
		else{
			ret = reinterpret_cast<PTable*>(pMemory::alloc());
			curr_lvl->entries[entry].setAddress(reinterpret_cast<uint64_t>(ret) >> 12);
			curr_lvl->entries[entry].setFlags(present | readNwrite | userSuper, true);
		}
		return ret;
	}

	PDEntry &Pagemap::virt2pte(uint64_t vaddr){
		size_t pml5_entry = (vaddr & ((uint64_t)0x1FF << 48)) >> 48;
		size_t pml4_entry = (vaddr & ((uint64_t)0x1FF << 39)) >> 39;
		size_t pml3_entry = (vaddr & ((uint64_t)0x1FF << 30)) >> 30;
		size_t pml2_entry = (vaddr & ((uint64_t)0x1FF << 21)) >> 21;
		size_t pml1_entry = (vaddr & ((uint64_t)0x1FF << 12)) >> 12;

		PTable *pml5, *pml4, *pml3, *pml2, *pml1;

		if (lvl5){
			pml5 = this->TOPLVL;
			pml4 = get_next_lvl(pml5, pml5_entry);
		}
		else{
			pml5 = nullptr;
			pml4 = this->TOPLVL;
		}
		pml3 = get_next_lvl(pml4, pml4_entry);
		pml2 = get_next_lvl(pml3, pml3_entry);
		pml1 = get_next_lvl(pml2, pml2_entry);
		return pml1->entries[pml1_entry];
	}

	void Pagemap::mapMem(uint64_t vaddr, uint64_t paddr, uint64_t flags){
		this->lock.lock();
		PDEntry &pml1_entry = this->virt2pte(vaddr);

		pml1_entry.setAddress(paddr >> 12);
		pml1_entry.setFlags(flags, true);
		this->lock.unlock();
	}

	void Pagemap::remapMem(uint64_t vaddr_old, uint64_t vaddr_new, uint64_t flags){
		this->lock.lock();
		PDEntry &pml1_entry = this->virt2pte(vaddr_old);

		uint64_t paddr = pml1_entry.getAddress() << 12;
		pml1_entry.value = 0;
		asm volatile ("invlpg (%0)" :: "r"(vaddr_old));
		this->lock.unlock();
		this->mapMem(vaddr_new, paddr, flags);
	}

	void Pagemap::mapUserMem(uint64_t vaddr, uint64_t paddr, uint64_t flags){
		this->mapMem(vaddr, paddr, flags | userSuper);
	}

	void Pagemap::mapHHMem(uint64_t paddr, uint64_t flags){
		this->mapMem(paddr + hhdm_tag->addr, paddr, flags);
	}

	void Pagemap::unmapMem(uint64_t vaddr){
		this->lock.lock();
		this->virt2pte(vaddr).value = 0;
		asm volatile ("invlpg (%0)" :: "r"(vaddr));
		this->lock.unlock();
	}

	void Pagemap::setFlags(uint64_t vaddr, uint64_t flags){
		this->lock.lock();
		this->virt2pte(vaddr).setFlags(flags, true);
		this->lock.unlock();
	}

	void Pagemap::remFlags(uint64_t vaddr, uint64_t flags){
		this->lock.lock();
		this->virt2pte(vaddr).setFlags(flags, false);
		this->lock.unlock();
	}

	void PDEntry::setFlag(PT_Flag flag, bool enabled){
		uint64_t bitSel = static_cast<uint64_t>(flag);
		this->value &= ~bitSel;
		if (enabled) this->value |= bitSel;
	}

	void PDEntry::setFlags(uint64_t flags, bool enabled){
		uint64_t bitSel = flags;
		this->value &= ~bitSel;
		if (enabled) this->value |= bitSel;
	}

	bool PDEntry::getFlag(PT_Flag flag){
		uint64_t bitSel = static_cast<uint64_t>(flag);
		return (this->value & (bitSel > 0)) ? true : false;
	}

	bool PDEntry::getFlags(uint64_t flags){
		return (this->value & (flags > 0)) ? true : false;
	}

	uint64_t PDEntry::getAddress(){
		return (this->value & 0x000FFFFFFFFFF000) >> 12;
	}

	void PDEntry::setAddress(uint64_t address){
		address &= 0x000000FFFFFFFFFF;
		this->value &= 0xFFF0000000000FFF;
		this->value |= (address << 12);
	}

	Pagemap *newPagemap(){
		Pagemap *pagemap = new Pagemap;

		if (kernel_pagemap == nullptr){
			pagemap->TOPLVL = reinterpret_cast<PTable*>(read_cr(3));
			return pagemap;
		}

		pagemap->TOPLVL = static_cast<PTable*>(pMemory::alloc());

		PTable *pml4 = pagemap->TOPLVL;
		PTable *kernel_pml4 = kernel_pagemap->TOPLVL;
		for (size_t i = 0; i < 512; i++){
			pml4->entries[i] = kernel_pml4->entries[i];
		}

		return pagemap;
	}

	Pagemap *clonePagemap(Pagemap *old){
		Pagemap *pagemap = new Pagemap;
		pagemap->TOPLVL = static_cast<PTable*>(pMemory::alloc());

		PTable *pml4 = pagemap->TOPLVL;
		PTable *old_pml4 = old->TOPLVL;
		for(size_t i = 0; i < 512; i++){
			pml4->entries[i] = old_pml4->entries[i];
		}

		return pagemap;
	}

	void switchPagemap(Pagemap *pmap){
		write_cr(3, reinterpret_cast<uint64_t>(pmap->TOPLVL));
	}

	PTable *getPagemap(){
		return reinterpret_cast<PTable*>(read_cr(3));
	}

	void init(){
		serial::log("Initialising VMM");

		if (isInit){
			serial::log("VMM has already been isInit!\n");
			return;
		}

		kernel_pagemap = newPagemap();
		switchPagemap(kernel_pagemap);

		serial::newline();
		isInit = true;
	}
}