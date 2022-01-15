#pragma once

#include <stdint.h>
#include <system/PCI/pcidesc.hpp>
#include <lib/TurboVector/TurboVector.hpp>

namespace turbo::pci {

	struct PCIdevice_t {
		uint16_t vendorID;
		uint16_t deviceID;
		uint16_t command;
		uint16_t status;
		uint8_t revisionID;
		uint8_t progIF;
		uint8_t subclass;
		uint8_t classID;
		uint8_t cacheLineSize;
		uint8_t latencyTimer;
		uint8_t headerType;
		uint8_t bist;
	};

	struct TranslatedPCIdevice_t {
		PCIdevice_t* device;
		const char* vendorStr;
		const char* deviceStr;
		const char* progIFStr;
		const char* subclassStr;
		const char* classStr;
		uint8_t bus;
		uint8_t dev;
		uint8_t func;
	};

	struct PCIheader0 {
		PCIdevice_t device;
		uint32_t BAR0;
		uint32_t BAR1;
		uint32_t BAR2;
		uint32_t BAR3;
		uint32_t BAR4;
		uint32_t BAR5;
		uint32_t cardbusCisPtr;
		uint16_t subsysVendorID;
		uint16_t subsysID;
		uint32_t expRomBaseAddr;
		uint8_t capabPtr;
		uint8_t rsv0;
		uint16_t rsv1;
		uint32_t rsv2;
		uint8_t intLine;
		uint8_t intPin;
		uint8_t minGrid;
		uint8_t maxLatency;
	};

	extern bool isInit;
	extern bool legacy;

	extern TurboVector<TranslatedPCIdevice_t*> PCIdevices;

	uint8_t readb(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset);
	void writeb(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset, uint8_t value);
	
	uint16_t readw(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset);
	void writew(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset, uint16_t value);

	uint32_t readl(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset);
	void writel(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset, uint32_t value);

	TranslatedPCIdevice_t* search(uint8_t classID, uint8_t subclass, uint8_t progIF, int skip);
	TranslatedPCIdevice_t* search(uint16_t vendorID, uint16_t deviceID, int skip);

	size_t count(uint8_t classID, uint8_t subclass, uint8_t progIF);
	size_t count(uint16_t vendorID, uint16_t deviceID);

	void init();
}
