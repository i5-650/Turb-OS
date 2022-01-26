#pragma once 

#include <lib/cpu/cpu.hpp>
#include <lib/lock.hpp>
#include <stdint.h>
#include <system/memory/heap/heap.hpp>
#include <lib/TurboVector/TurboVector.hpp>
#include <system/memory/vMemory/vMemory.hpp>
#include <drivers/fs/vfs/turboVFS.hpp>

using namespace turbo;


namespace turbo::scheduler {

    enum state_t {
        INITIAL_STATE,
        RUNNING,
        READY,
        BLOCKED,
        KILLED
    };

    struct process_t;
    // https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads
    struct thread_t {
        int TID;
        state_t state;
        uint8_t* threadStack;
        registers_t reg;
        process_t* parent;
        size_t sliceOfTime;
    };

    struct process_t {
        char name[128];
        int PID = 0;
        int nextTID;
        state_t state;
        TurboVector<thread_t*> threads;
        TurboVector<process_t*> children;
        process_t* parent;
        vMemory::Pagemap* processPagemap;
        turbo::vfs::tfs_node_t *current_dir;
    };

    extern bool isInit;
    extern process_t* initProc;

    extern size_t processesCounter;
    extern size_t threadsCounter;

    extern uint64_t timeSlice;

    // threads
    thread_t* allocThread(uint64_t address, uint64_t args);
    thread_t* createThread(uint64_t adress, uint64_t args, process_t* parent = nullptr);

    thread_t* getThisThread();

    void blockThread();
    void blockThread(thread_t* thread);

    void unblockThread(thread_t* thread);

    void exitThread();


    // processes
    process_t* allocProcess(const char* name, uint64_t address, uint64_t args);
    process_t* createProcess(const char* name, uint64_t address, uint64_t args);

    process_t* getThisProcess();

    void blockProcess();
    void blockProcess(process_t* process);

    void unblockProcess(process_t* process);

    void exitProcess();


    // global
    void switchTask(registers_t* reg);
    void init();
    void _yield(uint64_t mSeconds = 1);


    // basics gets
    static inline int getPID(){
        return getThisProcess()->PID;
    }

    static inline int getTID(){
        return getThisThread()->TID;
    }

    #define subSuccess(t, reg) \
    ({ \
        t->state = RUNNING;\
        *reg = t->reg;\
        vMemory::switchPagemap(getThisProcess()->processPagemap);\
        serial::log("[RUNNING] p[%d]->thread[%d]: CPU%zu\n", getThisProcess()->PID - 1, getThisThread()->TID - 1, thisCPU->lapicID);\
        schedLock.unlock();\
        _yield(timeSlice);\
        return;\
    })

    #define subNoFree(p, t, reg) \
    ({\
        cleanProcess(p);\
        if(thisCPU->currentProcess == nullptr){\
            thisCPU->idleP = allocProcess("IDLE", (uint64_t)_idle, 0);\
            threadsCounter--;\
        }\
        thisCPU->currentProcess = thisCPU->idleP;\
        thisCPU->currentThread = thisCPU->idleP->threads[0];\
        timeSlice = t->sliceOfTime;\
        t->state = RUNNING;\
            *reg  = t->reg;\
            vMemory::switchPagemap(p->processPagemap);\
            serial::log("[RUNNING] IDLE on CPU %zu", thisCPU->lapicID);\
            schedLock.unlock();\
            _yield();\
    })
}