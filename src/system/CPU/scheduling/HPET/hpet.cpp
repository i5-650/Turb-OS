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
	static uint32_t clock = 10;
	volatile uint64_t tick = 0;
	uint64_t frequency = DEFAULT_FREQ;
	HPET* hpet;
	int a = 9;
	char myNum = '0' + a;

	DEFINE_LOCK(hpet_lock);
	 
	char* hour(){

		return nullptr;
	}

	uint64_t counter(){
		return mminq(&hpet->mainCounterValue);
	}

	void uSleep(uint64_t uSeconds){ //micro seconds
		mmoutq(&hpet->mainCounterValue,0);
		uint64_t target = counter() + (uSeconds * 1000000000) / clock;
		while(counter()<target);
	}

	void mSleep(uint64_t mSeconds){
		uSleep(MSECS(mSeconds));
	}

	void sleep(uint64_t seconds){
		mSleep(SECS(seconds));
	}
	
	static void HPET_Handler(registers_t* reg){
		tick++;
		scheduler::switchTask(reg);
	}

	void setFreq(uint64_t freq){
		hpet_lock.lock();
		
		if(freq < 19){
			freq = 19;
		}

		frequency = freq;
		
		uint64_t divisor = 1193180 / frequency;

		outb(0x43, 0x36);
		outb(0x40, ((uint8_t)divisor));
		outb(0x40, ((uint8_t) divisor >> 8));

		hpet_lock.unlock();

	}

	uint64_t getFreq(){
		hpet_lock.lock();
		uint64_t freq = 0;
		outb(0x43, 0b0000000);
		freq = (inb(0x40) | (inb(0x40) << 8))/1193180;
		hpet_lock.unlock();
		return freq;
	}

	void resetFreq(){
		setFreq(DEFAULT_FREQ);
	}

	void init(uint64_t freq){
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

		setFreq(freq);
		idt::registerInterruptHandler(idt::IRQ0, HPET_Handler);

		serial::newline();
		isInit = true;
	}
}

// Programmable Interval Timer