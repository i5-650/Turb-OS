#include <lib/memory/mmio.hpp>
namespace turbo::system::CPU::scheduling::HPET{
    bool isInit false;
    static uint32_t clock = 0;
    HPET* hpet;

    uint64_t counter (){
        return mminq(&hpet->mainCounterValue);
    }
    void sleep (uint64_t uSeconds){//micro seconds
        uint64_t target = counter() + (uSeconds * 1000000000) / clock;
        while(counter()<target);
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
        hpet=(HPET*)(acpi::hpethdr->address.address);
        clock = hpet->generalCapabilities >> 32;
        mmoutq(&hpet->generalConfiguration,0);
        mmoutq(&hpet->mainCounterValue,0);
        mmoutq(&hpet->generalConfiguration,1);
        serial::newline();
        isInit=true;
    }
}

// Programmable Interval Timer