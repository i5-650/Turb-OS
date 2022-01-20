#pragma once 

#include <lib/cpu/cpu.hpp>
#include <lib/lock.hpp>
#include <stdint.h>
#include <system/memory/heap/heap.hpp>
#include <lib/TurboVector/TurboVector.hpp>
#include <system/memory/vMemory/vMemory.hpp>

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
}