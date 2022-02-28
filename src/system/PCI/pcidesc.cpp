#include <drivers/display/terminal/terminal.hpp>
#include <system/memory/heap/heap.hpp>
#include <lib/string.hpp>
#include <lib/memory/memory.hpp>


namespace turbo::pci {

	const char* deviceClasses[20] ={
		"Unclassified",
		"Mass Storage Controller",
		"Network Controller",
		"Display Controller",
		"Multimedia Controller",
		"Memory Controller",
		"Bridge Device",
		"Simple Communication Controller",
		"Base System Peripheral",
		"Input Device Controller",
		"Docking Station",
		"Processor",
		"Serial Bus Controller",
		"Wireless Controller",
		"Intelligent Controller",
		"Satellite Communication Controller",
		"Encryption Controller",
		"Signal Processing Controller",
		"Processing Accelerator",
		"Non Essential Instrumentation"
	};

	const char* getVendor(uint16_t vendorID){
		switch (vendorID){
		case 0x1234:
			return "QEMU";
		
		case 0x8086:
			return "Intel";

		case 0x1022:
			return "AMD";

		case 0x10DE:
			return "NVIDIA";
		}

		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", vendorID);
		return ret;
	}

	const char* getDeviceName(uint16_t vendorID, uint16_t deviceID){
		switch(vendorID){
			case 0x1234:
				if(deviceID == 0x1111){
					return "Virtual Video Controler";
				}
				break;
			
			case 0x8086:
				switch (deviceID){
					case 0x9b63:
						return "Host bridge";
					case 0xa3af:
						return "USB controller";
					case 0xa3b1:
						return "Signal Processing Controller";
					case 0xa3ba:
						return "Communication controller";
					case 0xa382:
						return "SATA controller";
					case 0xa3da:
						return "ISA bridge";
					case 0xa3a1:
						return "Memory controller";
					case 0xa3f0:
						return "Audio device";
					case 0xa3a3:
						return "System Management Bus";
					case 0x1911:
						return "Xeon E3-1200 v5/v6 / E3-1500 v5 / 6th/7th Gen Core Processor Gaussian Mixture Model";
					case 0x1901:
						return "Xeon E3-1200 v5/E3-1500 v5/6th Gen Core Processor PCIe Controller (x16)";
					case 0x0D55:
						return "Ethernet Connection (12) I219-V";
					case 0x29c0:
						return "82G33/G31/P35/P31 Express DRAM Controller";
					case 0x10d3:
						return "82574L Gigabit Network Connection";
					case 0x2934:
						return "82801I (ICH9 Family) USB UHCI Controller";
					case 0x2935:
						return "82801I (ICH9 Family) USB UHCI Controller";
					case 0x2936:
						return "82801I (ICH9 Family) USB UHCI Controller";
					case 0x293A:
						return "82801I (ICH9 Family) USB2 EHCI Controller";
					case 0x2918:
						return "82801IB (ICH9) LPC Interface Controller";
					case 0x2922:
						return "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller";
					case 0x2930:
						return "82801I (ICH9 Family) SMBus Controller";
				}

			break;
		}

		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", deviceID);
		return ret;
	}

	/**
	 * https://wiki.osdev.org/PCI#Class_Codes
	 * 
	 * the next functions will just do as the osdev wiki says  
	 * 
	 */
	const char* unclassifiedSubclassName(uint8_t subclassCode) {
		switch(subclassCode){
			case 0x00:
				return "Non-VGA-Compatible Unclassified";
			
			case 0x01:
				return "VGA-Compatible Unclassified";
		}

		char* ret = (char*) calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	// Mass Storage Class
	const char* mscSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "SCSI Bus Controller";
			case 0x01:
				return "IDE Controller";
			case 0x02:
				return "Floppy Disk Controller";
			case 0x03:
				return "IPI Bus Controller";
			case 0x04:
				return "RAID Controller";
			case 0x05:
				return "ATA Controller";
			case 0x06:
				return "Serial ATA";
			case 0x07:
				return "Serial Attached SCSI";
			case 0x08:
				return "Non-Volatile Memory Controller";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char* netSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Ethernet Controller";
			case 0x01:
				return "Token Ring Controller";
			case 0x02:
				return "FDDI Controller";
			case 0x03:
				return "ATM Controller";
			case 0x04:
				return "ISDN Controller";
			case 0x05:
				return "WorldFip Controller";
			case 0x06:
				return "PICMG 2.14 Multi Computing Controller";
			case 0x07:
				return "Infiniband Controller";
			case 0x08:
				return "Fabric Controller";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;	
	}

	const char* displaySubclassName(uint8_t subclassCode){
		switch (subclassCode)
		{
			case 0x00:
				return "VGA Compatible Controller";
			case 0x01:
				return "XGA Controller";
			case 0x02:
				return "3D Controller (Not VGA-Compatible)";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char* multimediaSubclassName(uint8_t subclassCode){
		switch (subclassCode)
		{
			case 0x00:
				return "Multimedia Video Controller";
			case 0x01:
				return "Multimedia Audio Controller";
			case 0x02:
				return "Computer Telephony Device";
			case 0x03:
				return "Audio Device";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char* memorySublassName(uint8_t subclassCode){
		switch (subclassCode)
		{
			case 0x00:
				return "RAM Controller";
			case 0x01:
				return "Flash Controller";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char* bridgeSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Host Bridge";
			case 0x01:
				return "ISA Bridge";
			case 0x02:
				return "EISA Bridge";
			case 0x03:
				return "MCA Bridge";
			case 0x04:
				return "PCI-to-PCI Bridge";
			case 0x05:
				return "PCMCIA Bridge";
			case 0x06:
				return "NuBus Bridge";
			case 0x07:
				return "CardBus Bridge";
			case 0x08:
				return "RACEway Bridge";
			case 0x09:
				return "PCI-to-PCI Bridge";
			case 0x0A:
				return "InfiniBand-to-PCI Host Bridge";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;		
	}

	const char *simpleCOMSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Serial Controller";
			case 0x01:
				return "Parallel Controller";
			case 0x02:
				return "Multiport Serial Controller";
			case 0x03:
				return "Modem";
			case 0x04:
				return "IEEE 488.1/2 (GPIB) Controller";
			case 0x05:
				return "Smart Card Controller";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *baseSysPerSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "PIC";
			case 0x01:
				return "DMA Controller";
			case 0x02:
				return "Timer";
			case 0x03:
				return "RTC Controller";
			case 0x04:
				return "PCI Hot-Plug Controller";
			case 0x05:
				return "SD Host controller";
			case 0x06:
				return "IOMMU";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *inputDevSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Keyboard Controller";
			case 0x01:
				return "Digitizer Pen";
			case 0x02:
				return "Mouse Controller";
			case 0x03:
				return "Scanner Controller";
			case 0x04:
				return "Gameport Controller";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *dockingStationSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Generic";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *procSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "386";
			case 0x01:
				return "486";
			case 0x02:
				return "Pentium";
			case 0x03:
				return "Pentium Pro";
			case 0x10:
				return "Alpha";
			case 0x20:
				return "PowerPC";
			case 0x30:
				return "MIPS";
			case 0x40:
				return "Co-Processor";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	// Serial Bus Controller
	const char *sbcSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "FireWire (IEEE 1394) Controller";
			case 0x01:
				return "ACCESS Bus";
			case 0x02:
				return "SSA";
			case 0x03:
				return "USB Controller";
			case 0x04:
				return "Fibre Channel";
			case 0x05:
				return "SMBus";
			case 0x06:
				return "Infiniband";
			case 0x07:
				return "IPMI Interface";
			case 0x08:
				return "SERCOS Interface (IEC 61491)";
			case 0x09:
				return "CANbus";
			case 0x80:
				return "SerialBusController - Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *wirelessSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "iRDA Compatible Controller";
			case 0x01:
				return "Consumer IR Controller";
			case 0x10:
				return "RF Controller";
			case 0x11:
				return "Bluetooth Controller";
			case 0x12:
				return "Broadband Controller";
			case 0x20:
				return "Ethernet Controller (802.1a)";
			case 0x21:
				return "Ethernet Controller (802.1b)";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *intelligentControlerSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "I20";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	// Satellite Communication Controller
	const char *sccSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Satellite TV Controller";
			case 0x01:
				return "Satellite Audio Controller";
			case 0x02:
				return "Satellite Voice Controller";
			case 0x03:
				return "Satellite Data Controller";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *encryptSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "Network and Computing Encrpytion/Decryption";
			case 0x10:
				return "Entertainment Encryption/Decryption";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	// Signal Processing Controller
	const char *spcSubclassName(uint8_t subclassCode){
		switch (subclassCode){
			case 0x00:
				return "DPIO Modules";
			case 0x01:
				return "Performance Counters";
			case 0x10:
				return "Communication Synchronizer";
			case 0x20:
				return "Signal Processing Management";
			case 0x80:
				return "Other";
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *getSubclassName(uint8_t classCode, uint8_t subclassCode){
		switch (classCode){
			case 0x00:
				return unclassifiedSubclassName(subclassCode);
			case 0x01:
				return mscSubclassName(subclassCode);
			case 0x02:
				return netSubclassName(subclassCode);
			case 0x03:
				return displaySubclassName(subclassCode);
			case 0x04:
				return multimediaSubclassName(subclassCode);
			case 0x05:
				return memorySublassName(subclassCode);
			case 0x06:
				return bridgeSubclassName(subclassCode);
			case 0x07:
				return simpleCOMSubclassName(subclassCode);
			case 0x08:
				return baseSysPerSubclassName(subclassCode);
			case 0x09:
				return inputDevSubclassName(subclassCode);
			case 0x0A:
				return dockingStationSubclassName(subclassCode);
			case 0x0B:
				return procSubclassName(subclassCode);
			case 0x0C:
				return sbcSubclassName(subclassCode);
			case 0x0D:
				return wirelessSubclassName(subclassCode);
			case 0x0E:
				return intelligentControlerSubclassName(subclassCode);
			case 0x0F:
				return sccSubclassName(subclassCode);
			case 0x10:
				return encryptSubclassName(subclassCode);
			case 0x11:
				return spcSubclassName(subclassCode);
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", subclassCode);
		return ret;
	}

	const char *getProgIFName(uint8_t classCode, uint8_t subclassCode, uint8_t progIF){
		switch (classCode){
			case 0x01:
				switch (subclassCode){
					case 0x01:
						switch (progIF)
						{
							case 0x00:
								return "ISA Compatibility mode-only controller";
							case 0x05:
								return "PCI native mode-only controller";
							case 0x0A:
								return "ISA Compatibility mode controller, supports both channels switched to PCI native mode";
							case 0x0F:
								return "PCI native mode controller, supports both channels switched to ISA compatibility mode";
							case 0x80:
								return "ISA Compatibility mode-only controller, supports bus mastering";
							case 0x85:
								return "PCI native mode-only controller, supports bus mastering";
							case 0x8A:
								return "ISA Compatibility mode controller, supports both channels switched to PCI native mode, supports bus mastering";
							case 0x8F:
								return "PCI native mode controller, supports both channels switched to ISA compatibility mode, supports bus mastering";
						}
						break;
					case 0x05:
						switch (progIF)
						{
							case 0x20:
								return "Single DMA";
							case 0x30:
								return "Chained DMA";
						}
						break;
					case 0x06:
						switch (progIF)
						{
							case 0x00:
								return "Vendor Specific Interface";
							case 0x01:
								return "AHCI 1.0";
							case 0x02:
								return "Serial Storage Bus";
						}
						break;
					case 0x07:
						switch (progIF)
						{
							case 0x00:
								return "SAS";
							case 0x01:
								return "Serial Storage Bus";
						}
						break;
					case 0x08:
						switch (progIF)
						{
							case 0x01:
								return "NVMHCI";
							case 0x02:
								return "NVM Express";
						}
						break;
				}
				break;
			case 0x03:
				switch (subclassCode)
				{
					case 0x00:
						switch (progIF)
						{
							case 0x00:
								return "VGA Controller";
							case 0x01:
								return "8514-Compatible Controller";
						}
						break;
				}
				break;
			case 0x06:
				switch (subclassCode)
				{
					case 0x04:
						switch (progIF)
						{
							case 0x00:
								return "Normal Decode";
							case 0x01:
								return "Subtractive Decode";
						}
						break;
					case 0x08:
						switch (progIF)
						{
							case 0x00:
								return "Transparent Mode";
							case 0x01:
								return "Endpoint Mode";
						}
						break;
					case 0x09:
						switch (progIF)
						{
							case 0x40:
								return "Semi-Transparent, Primary bus towards host CPU";
							case 0x80:
								return "Semi-Transparent, Secondary bus towards host CPU";
						}
						break;
				}
				break;
			case 0x07:
				switch (subclassCode){
					case 0x00:
						switch (progIF){
							case 0x00:
								return "8250-Compatible (Generic XT)";
							case 0x01:
								return "16450-Compatible";
							case 0x02:
								return "16550-Compatible";
							case 0x03:
								return "16650-Compatible";
							case 0x04:
								return "16750-Compatible";
							case 0x05:
								return "16850-Compatible";
							case 0x06:
								return "16950-Compatible";
						}
						break;
					case 0x01:
						switch (progIF)
						{
							case 0x00:
								return "Standard Parallel Port";
							case 0x01:
								return "Bi-Directional Parallel Port";
							case 0x02:
								return "ECP 1.X Compliant Parallel Port";
							case 0x03:
								return "IEEE 1284 Controller";
							case 0xFE:
								return "IEEE 1284 Target Device";
						}
						break;
					case 0x03:
						switch (progIF)
						{
							case 0x00:
								return "Generic Modem";
							case 0x01:
								return "Hayes 16450-Compatible Interface";
							case 0x02:
								return "Hayes 16550-Compatible Interface";
							case 0x03:
								return "Hayes 16650-Compatible Interface";
							case 0x04:
								return "Hayes 16750-Compatible Interface";
						}
						break;
				}
				break;
			case 0x08:
				switch (subclassCode)
				{
					case 0x00:
						switch (progIF)
						{
							case 0x00:
								return "Generic 8259-Compatible";
							case 0x01:
								return "ISA-Compatible";
							case 0x02:
								return "EISA-Compatible";
							case 0x10:
								return "I/O APIC Interrupt Controller";
							case 0x20:
								return "I/O(x) APIC Interrupt Controller";
						}
						break;
					case 0x01:
						switch (progIF)
						{
							case 0x00:
								return "Generic 8237-Compatible";
							case 0x01:
								return "ISA-Compatible";
							case 0x02:
								return "EISA-Compatible";
						}
						break;
					case 0x02:
						switch (progIF)
						{
							case 0x00:
								return "Generic 8254-Compatible";
							case 0x01:
								return "ISA-Compatible";
							case 0x02:
								return "EISA-Compatible";
							case 0x03:
								return "HPET";
						}
						break;
					case 0x03:
						switch (progIF)
						{
							case 0x00:
								return "Generic RTC";
							case 0x01:
								return "ISA-Compatible";
						}
						break;
				}
				break;
			case 0x09:
				switch (subclassCode)
				{
					case 0x04:
						switch (progIF)
						{
							case 0x00:
								return "Generic";
							case 0x10:
								return "Extended";
						}
						break;
				}
				break;
			case 0x0C:
				switch (subclassCode)
				{
					case 0x00:
						switch (progIF)
						{
							case 0x00:
								return "Generic";
							case 0x10:
								return "OHCI";
						}
						break;
					case 0x03:
						switch (progIF)
						{
							case 0x00:
								return "UHCI Controller";
							case 0x10:
								return "OHCI Controller";
							case 0x20:
								return "EHCI (USB2) Controller";
							case 0x30:
								return "XHCI (USB3) Controller";
							case 0x80:
								return "Unspecified";
							case 0xFE:
								return "USB Device (Not a Host Controller)";
						}
						break;
					case 0x07:
						switch (progIF)
						{
							case 0x00:
								return "SMIC";
							case 0x01:
								return "Keyboard Controller Style";
							case 0x02:
								return "Block Transfer";
						}
						break;
				}
				break;
		}
		char *ret = (char*)calloc(6, sizeof(char));
		sprintf(ret, "%.4X", progIF);
		return ret;
	}
	
}
