#include <lib/lock.hpp>

/**
__ATOMIC_ACQUIRE
    Creates an inter-thread happens-before constraint from the release (or stronger) 
    semantic store to this acquire load. 
    Can prevent hoisting of code to before the operation. 

__ATOMIC_RELEASE
    Creates an inter-thread happens-before constraint to acquire (or stronger) 
    semantic loads that read from this release store. 
    Can prevent sinking of code to after the operation. 

*/


void lock_t::lock(){
    while(__atomic_test_and_set(&this->locked, __ATOMIC_ACQUIRE));
}


void lock_t::unlock(){
    __atomic_clear(&this->locked, __ATOMIC_RELEASE);
}

bool lock_t::isLocked(){
    return this->locked;
}


/*
void acquire_lock(lock_t &lock){
    while (!__sync_bool_compare_and_swap(&lock, 0, 1)) while (lock) asm volatile ("pause");
}

void release_lock(lock_t &lock){
    __sync_lock_release(&lock);
}*/