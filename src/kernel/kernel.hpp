#pragma once
#include <stivale2.h>

#define STACK_SIZE 0x2000

extern uint8_t kernelStack[];

void *stivale2_get_tag(stivale2_struct *stivale, uint64_t id);