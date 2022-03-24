#include <drivers/display/serial/serial.hpp>
#include <cpuid.h>
#include <lib/portIO.hpp>
#include <lib/cpu/cpu.hpp>
#include <lib/memory/mmio.hpp>
#include <system/CPU/PIC/pic.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/ACPI/acpi.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <lib/lock.hpp>
#include <lib/cpu/cpu.hpp>
#include <system/CPU/scheduling/ohMyTime/omtime.hpp>

namespace turbo::apic {
	bool isInit = false;
	static bool x2apic = false;
	bool legacy = false;
	static uint64_t tickCountInMS = 0;

	static inline uint32_t reg2x2apic(uint32_t reg){
		uint32_t x2apic_reg = 0;

		if(reg == INTERRUPT_COMMAND_REGISTER){
			// 0x30 = begin of ICR
			x2apic_reg = 0x30;
		}
		else{
			x2apic_reg = reg >> 4;
		}
		return x2apic_reg + 0x800;
	}

	uint32_t lapicRead(uint32_t reg){
		if(x2apic){
			return rdmsr(reg2x2apic(reg));
		}

		return mmind(((void*)(acpi::lapicAddress + reg)));
	}

	void lapicWrite(uint32_t reg, uint32_t value){
		if(x2apic){
			wrmsr(reg2x2apic(reg), value);
		}
		else {
			mmoutd((void*)(acpi::lapicAddress + reg), value);
		}
	}

	static void lapicSetNMaskInt(uint8_t vec, uint8_t currentProcessorID, uint8_t processorID, uint16_t flags, uint8_t lint){
		if(processorID != 0xFF){
			if(currentProcessorID != processorID){
				return;
			}
		}

		// 0x400 = max val
		uint32_t nmi = 0x400 | vec;

		if (flags & 2){
			nmi |= 1 << 13;
		}

		if (flags & 8){
			nmi |= 1 << 15;
		}
		if (lint == 0){
			// LVT Local Interrupt 0 register
			lapicWrite(0x350, nmi);
		}
		else if (lint == 1){
			// LVT Local Interrupt 1 register
			lapicWrite(0x360, nmi);
		}
	}

	void lapicInit(uint8_t processorID){
		uint64_t apic_msr = rdmsr(0x1B) | (1 << 11);
		uint32_t a = 0, b = 0, c = 0, d = 0;

		if (__get_cpuid(1, &a, &b, &c, &d)){
			if (c & (1 << 21)){
				x2apic = true;
				apic_msr |= 1 << 10;
			}
		}

		wrmsr(0x1B, apic_msr);
		lapicWrite(0x80, 0);
		lapicWrite(0xF0, lapicRead(0xF0) | 0x100);
		if (!x2apic){
			lapicWrite(0xE0, 0xF0000000);
			// read Int Request Register
			lapicWrite(0xD0, lapicRead(0x20));
		}
		
		for (size_t i = 0; i < acpi::nmis.getLength(); i++){
			acpi::MADTnMaskInt *nmi = acpi::nmis[i];
			lapicSetNMaskInt(2, processorID, nmi->processor, nmi->flags, nmi->lint);
		}
	}

	uint32_t ioapicRead(uintptr_t ioapicAddress, size_t reg){
		mmoutd((void*)ioapicAddress, reg & 0xFF);
		return mmind((void*)(ioapicAddress + 16));
	}

	void ioapicWrite(uintptr_t ioapicAddress, size_t reg, uint32_t data){
		mmoutd((void*)ioapicAddress, reg & 0xFF);
		mmoutd((void*)(ioapicAddress + 16), data);
	}

	static uint32_t getGSICount(uintptr_t ioapicAddress){
		return (ioapicRead(ioapicAddress, 1) & 0xFF0000) >> 16;
	}

	static acpi::MADTIOApic *getIOapicGSI(uint32_t gsi){
		for (size_t i = 0; i < acpi::ioapics.getLength(); i++){
			acpi::MADTIOApic *ioapic = acpi::ioapics[i];
			
			if(ioapic->gsib <= gsi && ioapic->gsib + getGSICount(ioapic->addr) > gsi){
				return ioapic;
			}
		}
		return nullptr;
	}

	void ioapicRedirectGSI(uint32_t gsi, uint8_t vec, uint16_t flags){
		size_t io_apic = getIOapicGSI(gsi)->addr;

		uint32_t low_index = 0x10 + (gsi - getIOapicGSI(gsi)->gsib) * 2;
		uint32_t high_index = low_index + 1;

		uint32_t high = ioapicRead(io_apic, high_index);

		high &= ~0xFF000000;
		high |= ioapicRead(io_apic, 0) << 24;
		ioapicWrite(io_apic, high_index, high);

		uint32_t low = ioapicRead(io_apic, low_index);

		low &= ~(1 << 16);
		low &= ~(1 << 11);
		low &= ~0x700;
		low &= ~0xFF;
		low |= vec;

		if(flags & 2){
			low |= 1 << 13;
		}

		if(flags & 8){
			low |= 1 << 15;
		}

		ioapicWrite(io_apic, low_index, low);
	}

	void ioapicRedirectIRQ(uint32_t irq, uint8_t vector){
		for (size_t i = 0; i < acpi::isos.getLength(); i++){
			if (acpi::isos[i]->irqSource == irq){
				ioapicRedirectGSI(acpi::isos[i]->gsi, vector, acpi::isos[i]->flags);
				return;
			}
		}

		ioapicRedirectGSI(irq, vector, 0);
	}

	void apicSendIPI(uint32_t lapic_id, uint32_t flags){
		if(x2apic){
			wrmsr(0x830, ((uint64_t)lapic_id) << 32 | flags);
		}
		else{
			lapicWrite(INTERRUPT_COMMAND_REGISTER, (lapic_id << 24));
			lapicWrite(0x300, flags);
		}
	}

	void endOfInterrupt(){
		lapicWrite(0xB0, 0);
	}

	// if the timer is masked, we won't get the interrupt
	void lapicTimerMask(bool isMasked){
		if(isMasked){
			lapicWrite(LVT_TIMER_REGISTER, (lapicRead(LVT_TIMER_REGISTER) | (1 << 0x10)));
		}
		else{
			lapicWrite(LVT_TIMER_REGISTER, (lapicRead(LVT_TIMER_REGISTER) & ~(1 << 0x10)));
		}
	}

	void lapicTimerInit(){
		if(tickCountInMS == 0){
			// devide configuration register
			lapicWrite(DIVIDE_CONF_REGISTER, 0x03);
			// initial count register
			lapicWrite(INITIAL_COUNT_REGISTER, 0xFFFFFFFF);
			lapicTimerMask(false);
			// if we are too fast, it may creates issues
			omtime::mSleep(1);
			lapicTimerMask(true);
			tickCountInMS = (0xFFFFFFFF - lapicRead(CURRENT_COUNT_REGISTER));
		}
	}

	DEFINE_LOCK(lapicTimerLock);
	void lapicOneShot(uint8_t vector, uint64_t mSeconds){
		lapicTimerLock.lock();
		lapicTimerInit();
		// to have no interruption durint this part
		lapicTimerMask(true);

		lapicWrite(DIVIDE_CONF_REGISTER, 0x03);
		lapicWrite(LVT_TIMER_REGISTER, (((lapicRead(LVT_TIMER_REGISTER) & ~(0x03 << 17)) | (0x00 << 17)) & 0xFFFFFF00) | vector);
		lapicWrite(INITIAL_COUNT_REGISTER, (tickCountInMS * mSeconds));
		
		lapicTimerMask(false);
		// we change mask to listen to interrupt
		lapicTimerLock.unlock();
	}

	void lapicPeriodic(uint8_t vector, uint64_t mSeconds){
		lapicTimerLock.lock();
		lapicTimerInit();
		// to have no interruption durint this part
		lapicTimerMask(true);

		lapicWrite(DIVIDE_CONF_REGISTER, 0x03);
		//																					|-> the only difference
		lapicWrite(LVT_TIMER_REGISTER, (((lapicRead(LVT_TIMER_REGISTER) & ~(0x03 << 17)) | (0x01 << 17)) & 0xFFFFFF00) | vector);
		lapicWrite(INITIAL_COUNT_REGISTER, (tickCountInMS * mSeconds));
		
		lapicTimerMask(false);
		// we change mask to listen to interrupt
		lapicTimerLock.unlock();
	}

	uint16_t getSCIevent(){
		uint16_t a = 0, b = 0;
		
		if (acpi::fadthdr->PM1aEventBlock){
			a = inw(acpi::fadthdr->PM1aEventBlock);
			outw(acpi::fadthdr->PM1aEventBlock, a);
		}

		if (acpi::fadthdr->PM1bEventBlock){
			b = inw(acpi::fadthdr->PM1bEventBlock);
			outw(acpi::fadthdr->PM1bEventBlock, b);
		}

		return a | b;
	}

	void setSCIevent(uint16_t value){
		uint16_t a = acpi::fadthdr->PM1aEventBlock + (acpi::fadthdr->PM1EventLength / 2);
		uint16_t b = acpi::fadthdr->PM1bEventBlock + (acpi::fadthdr->PM1EventLength / 2);

		if(acpi::fadthdr->PM1aEventBlock){
			outw(a, value);
		}

		if(acpi::fadthdr->PM1bEventBlock){
			outw(b, value);
		}
	}

	static void SCIHandler(registers_t *){
		uint16_t event = getSCIevent();
		if (event & ACPI_POWER_BUTTON){
			acpi::shutdown();
			omtime::mSleep(50);
			outw(0xB004, 0x2000);
			outw(0x604, 0x2000);
			outw(0x4004, 0x3400);
		}
	}

	void init(){
		serial::log("[+] Initialising APIC");

		if(isInit){
			serial::log("Already init: APIC\n");
			return;
		}

		if(!acpi::madt || !acpi::madthdr){
			serial::log("MADT table not found!\n");
			return;
		}

		pic::disable();
		lapicInit(acpi::lapics[0]->processorID);

		setSCIevent(ACPI_POWER_BUTTON | ACPI_SLEEP_BUTTON | ACPI_WAKE);
		getSCIevent();

		idt::registerInterruptHandler(acpi::fadthdr->SCI_Interrupt + 32, SCIHandler);
		ioapicRedirectIRQ(acpi::fadthdr->SCI_Interrupt, acpi::fadthdr->SCI_Interrupt + 32);

		// COM1
		ioapicRedirectIRQ(4, 36);

		serial::newline();
		isInit = true;
	}
}
