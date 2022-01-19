#pragma once

#include <stddef.h>


struct lock_t {
    volatile bool locked = false;

    void lock();
    void unlock();
    bool isLocked();
};

#define DEFINE_LOCK(name) static lock_t name