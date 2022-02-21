#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <system/CPU/scheduling/PIT/pit.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <drivers/display/serial/serial.hpp>
#include <lib/portIO.hpp>
#include <lib/lock.hpp>

namespace turbo::pit{
	bool isInit = false;
	bool isScheduling = false;
	volatile uint64_t tick = 0;
	uint64_t frequence = PIT_DEFAULT_FREQUENCE;
	DEFINE_LOCK(pit_lock);

	void sleep(uint64_t seconds){
		uint64_t s = tick;
		while(tick < s + seconds * frequence);
	}

	void mSleep(uint64_t ms){
		uint64_t s = tick;
		while(tick < s + ms * (frequence / 100));
	}

	uint64_t getFrequence(){
		pit_lock.lock();
		uint64_t f = 0;
		outb(0x43, 0b0000000);
		f = inb(0x40);
		f |= inb(0x40) << 8;
		f = 1193180 / f;
		pit_lock.unlock();
		return f;
	}

	uint64_t getTick(){
		return tick;
	}
	
	void setFrequence(uint64_t f){
		pit_lock.lock();
		if(f < 19){
			frequence = 19;
		}
		frequence = f;

		outb(0x43, 0x36);

		outb(0x40, (uint8_t)(1193180/frequence));
		outb(0x40, (uint8_t)((1193180/frequence)>>8));
		pit_lock.unlock();
	}


	static void pitHandler(registers_t* regs){
		tick++;
		if(isScheduling){
			turbo::scheduler::switchTask(regs);
		}
	}

	void init(uint64_t f){
		turbo::serial::log("Initialising PIT...\n");

		if(isInit){
			turbo::serial::log("PIT: Already init ! \n");
			return;
		}

		setFrequence(f);
		turbo::idt::registerInterruptHandler(idt::IRQ0, pitHandler);

		isInit = true;
	}
}
