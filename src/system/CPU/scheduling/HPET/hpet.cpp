#include <lib/memory/mmio.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <system/ACPI/acpi.hpp>
#include <drivers/display/serial/serial.hpp>
#include <drivers/display/terminal/printf.h>
#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <lib/lock.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <lib/portIO.hpp>

using namespace turbo;

namespace turbo::hpet{
	bool isInit = false;
	static uint32_t clock = 0;
	HPET* hpet;

	DEFINE_LOCK(hpet_lock);
	 
	char* hour(){

		return nullptr;
	}

	uint64_t counter(){
		return mminq(&hpet->mainCounterValue);
	}

	void uSleep(uint64_t uSeconds){ //micro seconds
		uint64_t target = counter() + (uSeconds * 1000000000) / clock;
		while(counter()<target);
	}

	void mSleep(uint64_t mSeconds){
		uSleep(MSECS(mSeconds));
	}

	void sleep(uint64_t seconds){
		uSleep(SECS(seconds));
	}
	
	void init(){
		turbo::serial::log("HPET : init start\n");

		if (isInit){
		   turbo::serial::log("HPET : already initialised\n");
		   return; 
		}
		else if (!acpi::hpethdr){
			turbo::serial::log("HPET : table not found\n");
			return;
		}

		hpet = (HPET*)(acpi::hpethdr->address.address);
		clock = hpet->generalCapabilities >> 32; // divided by 2^32*

		mmoutq(&hpet->generalConfiguration,0);
		mmoutq(&hpet->mainCounterValue,0);
		mmoutq(&hpet->generalConfiguration,1);

		serial::newline();
		isInit = true;
	}
}
