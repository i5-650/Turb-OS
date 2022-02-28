#include <drivers/display/serial/serial.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <system/memory/pMemory/pMemory.hpp>
#include <system/memory/heap/heap.hpp>
#include <lib/memory/memory.hpp>
#include <lib/lock.hpp>
#include <lib/math.hpp>
#include <lib/panic.hpp>

Heap turboHeap;

HeapBlock *Heap::next(HeapBlock *block){
    return (HeapBlock*)(((uint8_t*)block) + block->size);
}

HeapBlock *Heap::split(HeapBlock *block, size_t size){
    if (block != nullptr && size != 0){
        while (size < block->size){
            size_t sz = block->size >> 1;
            block->size = sz;
            block = this->next(block);
            block->size = sz;
            block->free = true;
        }

        if(size <= block->size){
			return block;
		}
    }
    return nullptr;
}

HeapBlock *Heap::find_best(size_t size){
    if(size == 0){
		return nullptr;
	}

    HeapBlock *best_block = nullptr;
    HeapBlock *block = this->head;
    HeapBlock *tmp = this->next(block);

    if(tmp == this->tail && block->free){
		return this->split(block, size);
	}

    while (block < this->tail && tmp < this->tail){
        if (block->free && tmp->free && block->size == tmp->size){
            block->size <<= 1;
            if(size <= block->size && (best_block == nullptr || block->size <= best_block->size)){
				best_block = block;
			}

            block = this->next(tmp);
            if(block < this->tail){
				tmp = this->next(block);
			}
            continue;
        }

        if(block->free && size <= block->size && (best_block == nullptr || block->size <= best_block->size)){
			best_block = block;
		}

        if(tmp->free && size <= tmp->size && (best_block == nullptr || tmp->size < best_block->size)){
			best_block = tmp;
		}

        if(block->size <= tmp->size){
            block = this->next(tmp);
            if(block < this->tail){
				tmp = this->next(block);
			}
        }
        else{
            block = tmp;
            tmp = this->next(tmp);
        }
    }

    if(best_block != nullptr){
		return this->split(best_block, size);
	}

    return nullptr;
}

size_t Heap::required_size(size_t size){
    size_t actual_size = sizeof(HeapBlock);

    size += sizeof(HeapBlock);
    size = ALIGN_UP(size, sizeof(HeapBlock));

    while (size > actual_size){
		actual_size <<= 1;
	}

    return actual_size;
}

void Heap::coalescence(){
    while (true){
        HeapBlock *block = this->head;
        HeapBlock *tmp = this->next(block);

        bool no_coalescence = true;
        while(block < this->tail && tmp < this->tail){
            if(block->free && tmp->free && block->size == tmp->size){
                block->size <<= 1;
                block = this->next(block);
                if(block < this->tail){
                    tmp = this->next(block);
                    no_coalescence = false;
                }
            }
            else if(block->size < tmp->size){
                block = tmp;
                tmp = this->next(tmp);
            }
            else{
                block = this->next(tmp);
                if(block < this->tail){
					tmp = this->next(block);
				}
            }
        }

        if(no_coalescence){
			return;
		}
    }
}

void Heap::expand(size_t pagecount){
    this->lock.lock();
    INVALID(pagecount != 0, "HEAP: Page count can not be zero!");

    size_t size = ALIGN_UP_2(0x1000, (pagecount + this->pages) * 0x1000);
    INVALID(POWER_OF_2(size), "HEAP: Size is not power of two!");

    pagecount = size / 0x1000;

    this->data = turbo::pMemory::realloc(data, this->pages, pagecount);
    INVALID(this->data != nullptr, "HEAP: Could not allocate memory!");

    this->head = (HeapBlock*)this->data;
    this->head->size = size;
    this->head->free = true;

    this->tail = this->next(this->head);
    this->pages = pagecount;

    turbo::serial::log("HEAP: Expanded the heap. Current size: %zu bytes, %zu pages", size, pagecount);
    this->lock.unlock();
}

void Heap::setsize(size_t pagecount)
{
    INVALID(pagecount != 0, "HEAP: Page count can not be zero!");
    INVALID(pagecount > this->pages, "HEAP: Page count needs to be higher than current size!");
    pagecount = pagecount - this->pages;
    this->expand(pagecount);
}

void *Heap::malloc(size_t size){
    if(size == 0){
		return nullptr;
	}

    if(this->data == nullptr){
		this->expand(INIT_PAGES);
	}

    this->lock.lock();

    size_t actual_size = this->required_size(size);

    HeapBlock *found = this->find_best(actual_size);
    if(found == nullptr){
        this->coalescence();
        found = this->find_best(actual_size);
    }

    if(found != nullptr){
        turbo::serial::log("HEAP: Allocated %zu bytes", size);
        found->free = false;
        this->expanded = false;
        this->lock.unlock();
        return (void*)(((uint8_t*)(found)) + sizeof(HeapBlock));
    }

    if(this->expanded){
        turbo::serial::log("HEAP: Could not expand the heap!");
        this->expanded = false;
        this->lock.unlock();
        return nullptr;
    }
    this->lock.unlock();

    this->expand(size / 0x1000 + 1);
    this->expanded = true;
    return this->malloc(size);
}

void *Heap::calloc(size_t num, size_t size){
    void *ptr = this->malloc(num * size);
    if(!ptr){
		return nullptr;
	}

    memset(ptr, 0, num * size);
    return ptr;
}

void *Heap::realloc(void *ptr, size_t size){
    if(!ptr){
		return this->malloc(size);
	}

    HeapBlock *block = (HeapBlock*)(((uint8_t*)(ptr)) - sizeof(HeapBlock));
    size_t oldsize = block->size;

    if(size == 0){
        this->free(ptr);
        return nullptr;
    }

    if(size < oldsize){
		oldsize = size;
	}

    void *newptr = this->malloc(size);
    if (newptr == nullptr) return ptr;

    memcpy(newptr, ptr, oldsize);
    this->free(ptr);
    return newptr;
}

void Heap::free(void *ptr)
{
    if(this->data == nullptr){
		return;
	}

    if(ptr == nullptr){
		return;
	}

    INVALID(this->head <= ptr, "HEAP: Head is not smaller than pointer!");
    INVALID(ptr < this->tail, "HEAP: Pointer is not smaller than tail!");

    this->lock.lock();

    HeapBlock *block = (HeapBlock*)(((uint8_t*)(ptr)) - sizeof(HeapBlock));
    block->free = true;

    turbo::serial::log("HEAP: Freed %zu bytes", block->size - sizeof(HeapBlock));

    this->coalescence();

    this->lock.unlock();
}

size_t Heap::allocsize(void *ptr){
    if(this->data == nullptr){
		return 0;
	}

    if(!ptr){
		return 0;
	}

    return ((HeapBlock*)(((uint8_t*)(ptr)) - sizeof(HeapBlock)))->size - sizeof(HeapBlock);
}

void *operator new(size_t size){
    return malloc(size);
}

void *operator new[](size_t size){
    return malloc(size);
}

void operator delete(void *ptr){
    free(ptr);
}

void operator delete[](void *ptr){
    free(ptr);
}