#pragma once 

#include <stdint.h>
#include <stddef.h>

#define LAPIC_ID_REGISTER 0x020
#define LAPIC_VERSION_REGISTER 0x030
#define TASK_PRIORITY_REGISTER 0x080
#define ARBITRATION_PRIORITY_REGISTER 0x090
#define PROCESSOR_PRIORITY_REGISTER 0x0A0
#define EOI_REGISTER 0x0B0
#define REMOTE_READ_REGISTER 0x0C0
#define LOGICAL_DESTINATION_REGISTER 0x0E0
#define SPURIOUS_INT_VECTOR_REGISTER 0x0F0
#define ERROR_STATUS_REGISTER 0x280
#define INTERRUPT_COMMAND_REGISTER 0x310
#define LVT_TIMER_REGISTER 0x320
#define INITIAL_COUNT_REGISTER 0x380
#define CURRENT_COUNT_REGISTER 0x390
#define DIVIDE_CONF_REGISTER 0x3E0

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

	void lapicOneShot(uint8_t vector, uint64_t mSeconds = 1);
	void lapicPeriodic(uint8_t vector, uint64_t mSeconds = 1);

	void lapicInit(uint8_t processor_id);
	void init();
}