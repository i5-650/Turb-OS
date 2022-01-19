#pragma once

#include <stdint.h>

#define CPUID_INVARIANT_TSC (1 << 8)
#define CPUID_TSC_DEADLINE (1 << 24)
#define CPUID_SMEP (1 << 7)
#define CPUID_SMAP (1 << 20)
#define CPUID_UMIP (1 << 2)
#define CPUID_X2APIC (1 << 21)
#define CPUID_GBPAGE (1 << 26)

struct [[gnu::packed]] registers_t{
	// general purpose registers (that's what osdev says)
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	
	uint64_t rbp; //stack base pointer
	uint64_t rdi; //Destination  
	uint64_t rsi; //source
	uint64_t rdx; //Data
	uint64_t rcx; //counter
	uint64_t rbx; //Base
	uint64_t rax;//accumulator
	uint64_t int_no; 
	uint64_t error_code; 
	uint64_t rip; // register instruction pointer
	uint64_t cs; // code segment
	uint64_t rflags; 
	uint64_t rsp; // register stack pointer
	uint64_t ss; // stack segment
};

// MSR = Model Specifi Register
uint64_t rdmsr(uint32_t msr); // read
void wrmsr(uint32_t msr, uint64_t value); //write

void set_kernel_gs(uintptr_t addr);
void set_user_gs(uintptr_t addr);
void set_user_fs(uintptr_t addr);

// both are registers
uintptr_t get_user_gs();
uintptr_t get_user_fs();

// control register
void write_cr(uint64_t reg, uint64_t val);
uint64_t read_cr(uint64_t reg);

//extended control register
void wrxcr(uint32_t i, uint64_t value);

void xsave(void *region);
void xrstor(void *region);
void fxsave(void *region);
void fxrstor(void *region);

void enableSSE();
void enableSMEP();
void enableSMAP();
void enableUMIP();