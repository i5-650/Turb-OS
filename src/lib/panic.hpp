#pragma once

#include <stddef.h>

[[noreturn]] void panic(const char *message, const char *file, size_t line);
#define PANIC(b) (panic(b, __FILE__, __LINE__))

#define INVALID(x, msg) ({ \
    if (!(x)) \
    { \
        turbo::serial::log("%s", (msg)); \
        return; \
    } \
})