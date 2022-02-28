#include <drivers/display/terminal/terminal.hpp>
#include <drivers/display/serial/serial.hpp>
#include <system/memory/heap/heap.hpp>
#include <system/PCI/pci.hpp>
#include <lib/string.hpp>
#include <lib/portIO.hpp>
#include <system/ACPI/acpi.hpp>


namespace turbo::pci {

	bool isInit = false;
	bool legacy = false;

	TurboVector<TranslatedPCIdevice_t*> PCIdevices;

	static uint8_t currentBus, currentDev, currentFunc;
	static void getAddress(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset){
		uint32_t address = (bus << 16) | (dev << 11) | (func << 8) | (offset & ~((uint32_t)(3))) | 0x80000000;
		outl(0xcf8, address);
	}

	uint8_t readb(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset){
		getAddress(bus, dev, func, offset);
		return inb(0xcfc + (offset & 3));
	}

	void writeb(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset, uint8_t value){
		getAddress(bus, dev, func, offset);
		outb(0xcfc + (offset & 3), value);
	}

	uint16_t readw(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset){
		getAddress(bus, dev, func, offset);
		return inw(0xcfc + (offset & 3));
	}

	void writew(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset, uint16_t value){
		getAddress(bus, dev, func, offset);
		outw(0xcfc + (offset & 3), value);
	}

	uint32_t readl(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset){
		getAddress(bus, dev, func, offset);
		return inl(0xcfc + (offset & 3));
	}

	void writel(uint8_t bus, uint8_t dev, uint8_t func, uint32_t offset, uint32_t value){
		getAddress(bus, dev, func, offset);
		outl(0xcfc + (offset & 3), value);
	}

	TranslatedPCIdevice_t *search(uint8_t classID, uint8_t subclass, uint8_t progIF, int skip){
		if(!isInit){
			serial::log("[/!\\] PCI has not been initialised!\n");
			return nullptr;
		}
		for(uint64_t i = 0; i < PCIdevices.getLength(); ++i){
			if(PCIdevices[i]->device->classID == classID){
				if(PCIdevices[i]->device->subclass == subclass){
					if(PCIdevices[i]->device->progIF == progIF){
						if (skip > 0){
							--skip;
							continue;
						}
						else{
							return PCIdevices[i];
						}
					}
				}
			}
		}
		return nullptr;
	}

	TranslatedPCIdevice_t *search(uint16_t vendorID, uint16_t deviceID, int skip){
		if(!isInit){
			serial::log("[/!\\] PCI has not been initialised!\n");
			return nullptr;
		}

		for(uint64_t i = 0; i < PCIdevices.getLength(); ++i){
			if(PCIdevices[i]->device->vendorID == vendorID){
				if(PCIdevices[i]->device->deviceID == deviceID){
					if(skip > 0){
						--skip;
						continue;
					}
					else{
						return PCIdevices[i];
					}
				}
			}
		}
		return nullptr;
	}

	size_t count(uint16_t vendorID, uint16_t deviceID){
		if(!isInit){
			serial::log("[/!\\] PCI has not been initialised!\n");
			return 0;
		}
		size_t num = 0;
		for(uint64_t i = 0; i < PCIdevices.getLength(); ++i){
			if(PCIdevices[i]->device->vendorID == vendorID){
				if(PCIdevices[i]->device->deviceID == deviceID){
					++num;
				}
			}
		}
		return num;
	}

	size_t count(uint8_t classID, uint8_t subclass, uint8_t progIF){
		if (!isInit){
			serial::log("[/!\\] PCI has not been initialised!\n");
			return 0;
		}

		size_t num = 0;
		for(uint64_t i = 0; i < PCIdevices.getLength(); ++i){
			if(PCIdevices[i]->device->classID == classID){
				if(PCIdevices[i]->device->subclass == subclass){
					if(PCIdevices[i]->device->progIF == progIF){
						num++;
					}
				}
			}
		}
		return num;
	}

	TranslatedPCIdevice_t *translate(PCIdevice_t* device){
		TranslatedPCIdevice_t *pcidevice = (TranslatedPCIdevice_t*)malloc(sizeof(TranslatedPCIdevice_t));

		pcidevice->device = device;

		pcidevice->vendorStr = getVendor(device->vendorID);
		pcidevice->deviceStr = getDeviceName(device->vendorID, device->deviceID);
		pcidevice->progIFStr = getProgIFName(device->classID, device->subclass, device->progIF);
		pcidevice->subclassStr = getSubclassName(device->classID, device->subclass);
		pcidevice->classStr = deviceClasses[device->classID];

		pcidevice->bus = currentBus;
		pcidevice->dev = currentDev;
		pcidevice->func = currentFunc;

		return pcidevice;
	}

	void enumfunc(uint64_t devAddress, uint64_t func){
		uint64_t offset = func << 12;
		uint64_t funcAddress = devAddress + offset;

		PCIdevice_t *pcidevice = (PCIdevice_t*)funcAddress;
		if(pcidevice->deviceID == 0 || pcidevice->deviceID == 0xFFFF) {
			return;
		}

		currentFunc = func;

		PCIdevices.push_back(translate(pcidevice));
		serial::log("%.4X:%.4X %s %s",
			PCIdevices.last()->device->vendorID,
			PCIdevices.last()->device->deviceID,
			PCIdevices.last()->vendorStr,
			PCIdevices.last()->deviceStr);
	}

	void enumdevice(uint64_t busAddress, uint64_t dev)
	{
		uint64_t offset = dev << 15;
		uint64_t devAddress = busAddress + offset;

		PCIdevice_t *pcidevice = (PCIdevice_t*)devAddress;
		if (pcidevice->deviceID == 0 || pcidevice->deviceID == 0xFFFF){
			return;
		}

		currentDev = dev;

		for (uint64_t func = 0; func < 8; ++func){
			enumfunc(devAddress, func);
		}
	}

	void enumbus(uint64_t baseAddress, uint64_t bus){
		uint64_t offset = bus << 20;
		uint64_t busAddress = baseAddress + offset;

		PCIdevice_t *pcidevice = (PCIdevice_t*)busAddress;
		
		if(pcidevice->deviceID == 0 || pcidevice->deviceID == 0xFFFF){
			return;
		}

		currentBus = bus;

		for (uint64_t dev = 0; dev < 32; ++dev){
			enumdevice(busAddress, dev);
		}
	}

	void init(){
		turbo::serial::log("[+] Initialising PCI");

		if(isInit){
			turbo::serial::log("[!!] Already init: PCI!\n");
			return;
		}

		if(!turbo::acpi::mcfghdr){
			turbo::serial::log("MCFG not found");
			legacy = false;
		}


		PCIdevices.init(5);

		if(!legacy){
			int entries = ((turbo::acpi::mcfghdr->header.length) - sizeof(turbo::acpi::MCFGHeader)) / sizeof(turbo::acpi::deviceconfig);
			for(int i = 0; i < entries; ++i){
				turbo::acpi::deviceconfig *newDeviceConfig = (turbo::acpi::deviceconfig*)((uint64_t)turbo::acpi::mcfghdr + sizeof(turbo::acpi::MCFGHeader) + (sizeof(turbo::acpi::deviceconfig) * i));
				for(uint64_t bus = newDeviceConfig->startbus; bus < newDeviceConfig->endbus; bus++){
					enumbus(newDeviceConfig->baseaddr, bus);
				}
			}
		}
		else{
			for (int bus = 0; bus < 256; ++bus){
				for (int dev = 0; dev < 32; ++dev){
					for (int func = 0; func < 8; ++func){
						uint32_t config_0 = readl(bus, dev, func, 0);

						if(config_0 == 0xFFFFFFFF){
							continue;
						}

						uint32_t config_4 = readl(bus, dev, func, 0x4);
						uint32_t config_8 = readl(bus, dev, func, 0x8);
						uint32_t config_c = readl(bus, dev, func, 0xc);

						PCIdevice_t *pcidevice = (PCIdevice_t*)malloc(sizeof(PCIdevice_t));

						pcidevice->vendorID = (uint16_t)config_0;
						pcidevice->deviceID = (uint16_t)(config_0 >> 16);
						pcidevice->command = (uint16_t)config_4;
						pcidevice->status = (uint16_t)(config_4 >> 16);
						pcidevice->revisionID = (uint8_t)config_8;
						pcidevice->progIF = (uint8_t)(config_8 >> 8);
						pcidevice->subclass = (uint8_t)(config_8 >> 16);
						pcidevice->classID = (uint8_t)(config_8 >> 24);
						pcidevice->cacheLineSize = (uint8_t)config_c;
						pcidevice->latencyTimer = (uint8_t)(config_c >> 8);
						pcidevice->headerType = (uint8_t)(config_c >> 16);
						pcidevice->bist = (uint8_t)(config_c >> 24);

						currentBus = bus;
						currentDev = dev;
						currentFunc = func;

						PCIdevices.push_back(translate(pcidevice));

						free(pcidevice);

						turbo::serial::log("%.4X:%.4X %s %s",
							PCIdevices.last()->device->vendorID,
							PCIdevices.last()->device->deviceID,
							PCIdevices.last()->vendorStr,
							PCIdevices.last()->deviceStr);
					}
				}
			}
		}
		
		turbo::serial::newline();
		isInit = true;
	}
}