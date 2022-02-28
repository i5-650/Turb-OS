#include <drivers/display/serial/serial.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <kernel/main.hpp>
#include <lib/memory/memory.hpp>
#include <stivale2.h>
#include <lib/lock.hpp>
#include <lib/math.hpp>
#include <kernel/kernel.hpp>

namespace turbo::pMemory {
	Bitmap bitmap;
	bool isInit = false;
	static uintptr_t highPage = 0;
	static size_t last = 0;
	static size_t usedRam = 0;
	static size_t freeRam = 0;

	DEFINE_LOCK(mmLock);

	static void* ohMyAlloc(size_t count, size_t limit){
		size_t p = 0;
		while(last < limit){
			if(!bitmap[last++]){
				if(++p == count){
					size_t page = last - count;
					for(size_t i = page; i < last; ++i){
						bitmap.set(i, true);
					}
					return ((void*) (page * 0x1000));
				}
			}
			else {
				p = 0;
			}
		}
		return nullptr;
	}

	void* alloc(size_t count){
		mmLock.lock();
		size_t tmp = last;
		void* ret = ohMyAlloc(count, highPage / 0x1000);
		if(!ret){
			last = 0;
			ret = ohMyAlloc(count, tmp);
		}

		memset(ret, 0, count * 0x1000);
		usedRam += count * 0x1000;
		freeRam -= count * 0x1000;
		mmLock.unlock();
		return ret;
	}

	void free(void* ptr, size_t count){
		if(!ptr){
			return;
		}
		mmLock.lock();
		size_t page = ((size_t)ptr) / 0x1000;
		for(size_t i = page; i < page + count; i++){
			bitmap.set(i, false);
		}
		if(last > page){
			last = page;
		}

		usedRam -= count * 0x1000;
		freeRam += count * 0x1000;
		mmLock.unlock();
	}

	void* realloc(void* ptr, size_t oldSize, size_t newSize){
		if(!ptr){
			serial::log("j'aime");
			return alloc(newSize);
		}
		
		if(!newSize){
			free(ptr, oldSize);
			return nullptr;
		}

		usedRam = usedRam - oldSize * 0x1000 + newSize * 0x1000;
		freeRam = freeRam + oldSize * 0x1000 - newSize * 0x1000;

		if(newSize < oldSize){
			oldSize = newSize;
		}

		void* newFrag = alloc(newSize);
		memcpy(newFrag, ptr, oldSize);
		free(ptr);
		return newFrag;
	}

	size_t getFreeRam(){
		return freeRam;
	}

	size_t getUsedRam(){
		return usedRam;
	}


	void init(){
		serial::log("[+] Initialising pMemory");
		if(isInit){
			serial::log("[!!] Already Init");
			return;
		}

		for(size_t i = 0; i < mmap_tag->entries; i++){
			if(mmap_tag->memmap[i].type != STIVALE2_MMAP_USABLE){
				continue;
			}

			uintptr_t top = mmap_tag->memmap[i].base + mmap_tag->memmap[i].length;
			freeRam += mmap_tag->memmap[i].length;

			if(top > highPage){
				highPage = top;
			}
		}

		size_t bitmapSize = ALIGN_UP((highPage / 0x1000) / 8, 0x1000);

		for(size_t i = 0; i < mmap_tag->entries; i++){
			if(mmap_tag->memmap[i].type != STIVALE2_MMAP_USABLE){
				continue;
			}

			if(mmap_tag->memmap[i].length >= bitmapSize){
				bitmap.buffer = (uint8_t*) mmap_tag->memmap[i].base;
				memset(bitmap.buffer, 0xFF, bitmapSize);
				mmap_tag->memmap[i].length -= bitmapSize;
				mmap_tag->memmap[i].base += bitmapSize;
				freeRam -= bitmapSize;
				break;
			}
		}

		for(size_t i = 0; i < mmap_tag->entries; i++){
			if(mmap_tag->memmap[i].type != STIVALE2_MMAP_USABLE){
				continue;
			}

			for(uintptr_t j = 0; j < mmap_tag->memmap[i].length; j += 0x1000){
				bitmap.set((mmap_tag->memmap[i].base + j) / 0x1000, false);
			}
		}

		serial::newline();
		isInit = true;
	}

}