#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/CPU/SMP/smp.hpp>
#include <kernel/kernel.hpp>
#include <drivers/display/serial/serial.hpp>
#include <lib/string.hpp>
#include <lib/TurboVector/TurboVector.hpp>
#include <lib/lock.hpp>

using namespace turbo;

namespace turbo::scheduler {
    bool isInit = false;
    
    static uint64_t nextPID = 1;
    static uint8_t schedulerVector = 0;

    TurboVector<process_t*> processTable;

    process_t* initProc = nullptr;

    size_t processesCounter = 0;
    size_t threadsCounter = 0;

    // defines locks
    DEFINE_LOCK(threadLock);
    DEFINE_LOCK(schedLock);
    DEFINE_LOCK(processLock);

    // threads
    thread_t* allocThread(uint64_t address, uint64_t args){
        threadLock.lock();

        thread_t* myThread = new thread_t;
        myThread->state = INITIAL_STATE;

        threadLock.unlock();
        return ;
    }
}