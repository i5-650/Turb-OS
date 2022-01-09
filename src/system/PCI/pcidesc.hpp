#pragma once

#include <stdint.h>

namespace turbo::pci {
	
	extern const char *deviceClasses[20];

	const char* getVendor(uint16_t vendorID);
	const char* getDeviceName(uint16_t vendorID, uint16_t deviceID);
	const char* getSubclassName(uint8_t classCode, uint8_t subclassCode);
	const char* getProgIFName(uint8_t classCode, uint8_t subclassCode, uint8_t progIF);

}