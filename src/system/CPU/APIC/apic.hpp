#pragma once 

#include <stdint.h>
#include <stddef.h>

namespace turbo::apic {
	enum events{
		ACPI_TIMER = 0x0001,
		ACPI_BUSMASTER = 0x0010,
		ACPI_GLOBAL = 0x0020,
		ACPI_POWER_BUTTON = 0x0100,
		ACPI_SLEEP_BUTTON = 0x0200,
		ACPI_RTC_ALARM = 0x0400,
		ACPI_PCIE_WAKE = 0x4000,
		ACPI_WAKE = 0x8000
	};

	extern bool isInit;

	uint32_t lapicRead(uint32_t reg);
	void lapicWrite(uint32_t reg, uint32_t value);

	uint32_t ioAPICRead(uintptr_t ioapic_address, size_t reg);
	void ioAPICWrite(uintptr_t ioapic_address, size_t reg, uint32_t data);

	void ioapicRedirectGSI(uint32_t gsi, uint8_t vec, uint16_t flags);
	void ioapicRedirectIRQ(uint32_t irq, uint8_t vect);
	void apicSendIPI(uint32_t lapic_id, uint32_t flags);
	void endOfInterrupt();

	void lapicInit(uint8_t processor_id);
	void init();
}