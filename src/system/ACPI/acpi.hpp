#pragma once

#include <lib/TurboVector/TurboVector.hpp>
#include <stdint.h>
#include <stddef.h>

namespace turbo::acpi {

    #define ACPI_ENABLED 0x0001
    #define ACPI_SLEEP 0x2000

    #define ACPI_GAS_MMIO 0
    #define ACPI_GAS_IO 1
    #define ACPI_GAS_PCI 2

    struct [[gnu::packed]] RSDP{
        unsigned char signature[8];
        uint8_t chksum;
        uint8_t oemid[6];
        uint8_t revision;
        uint32_t rsdtaddress;
        uint32_t length;
        uint64_t xsdtaddress;
        uint8_t extchksum;
        uint8_t reserved[3];
    };

    struct [[gnu::packed]] SDTHeader{
        unsigned char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t chksum;
        uint8_t oemid[6];
        uint8_t oemtableid[8];
        uint32_t oemrevision;
        uint32_t creatID;
        uint32_t creatreVision;
    };

    struct [[gnu::packed]] MCFGHeader{
        struct SDTHeader header;
        uint64_t reserved;
    };

    struct [[gnu::packed]] MADTHeader{
        struct SDTHeader sdt;
        uint32_t localControllerAddress;
        uint32_t flags;
        char entriesBegin[];
    };

    struct [[gnu::packed]] MADT{
        uint8_t type;
        uint8_t length;
    };

    struct [[gnu::packed]] MADTLapic{
        struct MADT madtHeader;
        uint8_t processorID;
        uint8_t apicID;
        uint32_t flags;
    };

    struct [[gnu::packed]] MADTIOApic{
        MADT madtHeader;
        uint8_t apicID;
        uint8_t reserved;
        uint32_t addr;
        uint32_t gsib;
    };

    struct [[gnu::packed]] MADTIso{
        struct MADT madtHeader;
        uint8_t busSource;
        uint8_t irqSource;
        uint32_t gsi;
        uint16_t flags;
    };

    struct [[gnu::packed]] MADTnMaskInt{
        struct MADT madtHeader;
        uint8_t processor;
        uint16_t flags;
        uint8_t lint;
    };

    struct [[gnu::packed]] GenericAddressStructure{
        uint8_t addressSpace;
        uint8_t bitWidth;
        uint8_t bitOffset;
        uint8_t accessSize;
        uint64_t address;
    };

    struct [[gnu::packed]] HPETHeader{
        struct SDTHeader header;
        uint8_t hardware_rev_id;
        uint8_t comparator_count : 5;
        uint8_t counter_size : 1;
        uint8_t reserved : 1;
        uint8_t legacy_replacement : 1;
        uint16_t pci_vendor_id;
        struct GenericAddressStructure address;
        uint8_t hpet_number;
        uint16_t minimum_tick;
        uint8_t page_protection;
    };

    struct [[gnu::packed]] FADTHeader{
        struct SDTHeader header;
        uint32_t FirmwareCtrl;
        uint32_t Dsdt;
        uint8_t Reserved;
        uint8_t PreferredPowerManagementProfile;
        uint16_t SCI_Interrupt;
        uint32_t SMI_CommandPort;
        uint8_t AcpiEnable;
        uint8_t AcpiDisable;
        uint8_t S4BIOS_REQ;
        uint8_t PSTATE_Control;
        uint32_t PM1aEventBlock;
        uint32_t PM1bEventBlock;
        uint32_t PM1aControlBlock;
        uint32_t PM1bControlBlock;
        uint32_t PM2ControlBlock;
        uint32_t PMTimerBlock;
        uint32_t GPE0Block;
        uint32_t GPE1Block;
        uint8_t PM1EventLength;
        uint8_t PM1ControlLength;
        uint8_t PM2ControlLength;
        uint8_t PMTimerLength;
        uint8_t GPE0Length;
        uint8_t GPE1Length;
        uint8_t GPE1Base;
        uint8_t CStateControl;
        uint16_t WorstC2Latency;
        uint16_t WorstC3Latency;
        uint16_t FlushSize;
        uint16_t FlushStride;
        uint8_t DutyOffset;
        uint8_t DutyWidth;
        uint8_t DayAlarm;
        uint8_t MonthAlarm;
        uint8_t Century;
        uint16_t BootArchitectureFlags;
        uint8_t Reserved2;
        uint32_t Flags;
        struct GenericAddressStructure ResetReg;
        uint8_t ResetValue;
        uint8_t Reserved3[3];
        uint64_t X_FirmwareControl;
        uint64_t X_Dsdt;
        struct GenericAddressStructure X_PM1aEventBlock;
        struct GenericAddressStructure X_PM1bEventBlock;
        struct GenericAddressStructure X_PM1aControlBlock;
        struct GenericAddressStructure X_PM1bControlBlock;
        struct GenericAddressStructure X_PM2ControlBlock;
        struct GenericAddressStructure X_PMTimerBlock;
        struct GenericAddressStructure X_GPE0Block;
        struct GenericAddressStructure X_GPE1Block;
    };

    struct [[gnu::packed]] deviceconfig{
        uint64_t baseaddr;
        uint16_t pciseggroup;
        uint8_t startbus;
        uint8_t endbus;
        uint32_t reserved;
    };

    extern bool isInit;
    extern bool madt;

    extern bool useXSTD;
    extern struct RSDP *rsdp;

    extern struct MCFGHeader *mcfghdr;
    extern struct MADTHeader *madthdr;
    extern struct FADTHeader *fadthdr;
    extern struct HPETHeader *hpethdr;
    extern struct SDTHeader *rsdt;

    extern TurboVector<MADTLapic*> lapics;
    extern TurboVector<MADTIOApic*> ioapics;
    extern TurboVector<MADTIso*> isos;
    extern TurboVector<MADTnMaskInt*> nmis;

    extern uint32_t *SMI_CMD;
    extern uint8_t ACPI_ENABLE;
    extern uint8_t ACPI_DISABLE;
    extern uint32_t PM1a_CNT;
    extern uint32_t PM1b_CNT;
    extern uint16_t SLP_TYPa;
    extern uint16_t SLP_TYPb;
    extern uint16_t SLP_EN;
    extern uint16_t SCI_EN;
    extern uint8_t PM1_CNT_LEN;

    extern uintptr_t lapicAddress;

    void init();

    void shutdown();
    void reboot();

    void *findTable(const char *signature, size_t skip = 0);
}