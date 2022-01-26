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
#include <system/memory/heap/heap.hpp>
#include <lib/panic.hpp>
#include <drivers/display/terminal/printf.h>
#include <drivers/fs/vfs/turboVFS.hpp>

using namespace turbo;
using namespace turbo::heap;

#define DEFAULT_TIMESLICE 5


namespace turbo::scheduler {
    bool isInit = false;
    
    static uint64_t nextPID = 1;
    static uint8_t schedulerVector = 0;

    TurboVector<process_t*> processTable;

    process_t* initProc = nullptr;

    size_t processesCounter = 0;
    size_t threadsCounter = 0;

    uint64_t timeSlice;

    // defines locks
    DEFINE_LOCK(threadLock);
    DEFINE_LOCK(schedLock);
    DEFINE_LOCK(processLock);

    // usefull functions
    void _idle(){
        while(true){
            asm volatile("hlt");
        }
    }

    void _yield(uint64_t mSeconds){
        if(apic::isInit){
            apic::lapicOneShot(schedulerVector, mSeconds);
        }
        else {
            PANIC("_YIELD EXCEPTION");
        }
    }

    // threads
    thread_t* allocThread(uint64_t address, uint64_t args){
        threadLock.lock();

        thread_t* myThread = (thread_t*)malloc(sizeof(thread_t));

        myThread->state = INITIAL_STATE;
        myThread->threadStack = (uint8_t*)malloc(STACK_SIZE);

        myThread->reg.rflags = 0x202;
        myThread->reg.cs = 0x28;
        myThread->reg.ss = 0x30;
        myThread->reg.rip = address;
        myThread->reg.rdi = (uint64_t)(args);
        myThread->reg.rsp = (uint64_t)(myThread->threadStack + STACK_SIZE);
        
        myThread->parent = nullptr;

        threadLock.unlock();
        return myThread;
    }

    thread_t* createThread(uint64_t address, uint64_t args, process_t* parent){
        thread_t* myThread = allocThread(address, args);
        
        if(parent){
            myThread->TID = parent->nextTID++;
            myThread->parent = parent;
            parent->threads.push_back(myThread);
        }

        threadsCounter++;
        threadLock.lock();
        myThread->state = READY;
        threadLock.unlock();

        return myThread;
    }

    thread_t* getThisThread(){
        asm volatile("cli");
        thread_t* t = thisCPU->currentThread;
        asm volatile("sti");
        return t;
    }

    void blockThread(){
        asm volatile("cli");
        
        if(getThisThread()->state == READY || getThisThread()->state == RUNNING){
            getThisThread()->state = BLOCKED;
            // debug purpose
            serial::log("[BLOCK] TID: %d PID: %d\n", getThisThread()->TID, getThisProcess()->PID);
        }

        asm volatile("sti");
    }

    void blockThread(thread_t* t){
        asm volatile("cli");
        
        if(t->state == READY || t->state == RUNNING){
            t->state = BLOCKED;
            // debug purpose
            serial::log("[BLOCK] TID: %d PID: %d\n", t->TID, t->parent->PID);
        }

        asm volatile("sti");
    }

    void unblockThread(thread_t* t){
        asm volatile("cli");

        if(t->state == BLOCKED){
            t->state = READY;

            serial::log("[UNBLOCK] TID: %d\n", t->TID);
        }

        asm volatile("sti");
    }

    void exitThread(){
        asm volatile("cli");

        if(getThisProcess() == initProc 
        && getThisProcess()->threads.getLength() == 1 
        && getThisProcess()->children.getLength() == 0){
            serial::log("[ERROR] init proc can't be killed ! \n");
        }

        getThisThread()->state = KILLED;
        serial::log("[EXIT] PID: %d", getThisProcess()->PID);
        asm volatile("sti");
        _yield();
        while(true){
            asm volatile("hlt");
        }
    }

    // processes
    process_t* allocProcess(const char* name, uint64_t address, uint64_t args){
        process_t* p = new process_t;

        processLock.lock();
        strncpy(p->name, name, (strlen(name) < 128) ? strlen(name) : 128);
        p->PID = nextPID++;
        p->state = INITIAL_STATE;
        p->processPagemap = vMemory::newPagemap();
        p->current_dir = turbo::vfs::open(NULL,"/");
        p->parent = nullptr;

        if(address){
            createThread(address, args, p);
        }

        if(isInit){
            initProc = p;
            isInit = true;
        }

        processLock.unlock();

        return p;
    }

    process_t* createProcess(const char* name, uint64_t address, uint64_t args){
        process_t* p = allocProcess(name, address, args);

        processTable.push_back(p);
        processesCounter++;
        processLock.lock();
        p->state = READY;
        processLock.unlock();

        return p;
    }

    process_t* getThisProcess(){
        asm volatile("cli");
        process_t* p = thisCPU->currentProcess;
        asm volatile("sti");

        return p;
    }

    void blockProcess(){
        asm volatile("cli");
        if(getThisProcess() == initProc){
            serial::log("[BLOCK] init process can't be blocked\n");
            asm volatile("sti");
            return;
        }

        if(getThisProcess()->state == READY || getThisProcess()->state == RUNNING){
            getThisProcess()->state = BLOCKED;
            serial::log("[BLOCK] PID: %d", getThisProcess()->PID);
        }

        asm volatile("sti");
        _yield();
    }

    void blockProcess(process_t* p){
        asm volatile("cli");

        if(p->state == READY || p->state == RUNNING){
            p->state = BLOCKED;
            serial::log("[BLOCK] PID: %d", p->PID);
        }

        asm volatile("sti");
    }

    void unblockProcess(process_t* p){
        asm volatile("cli");
        if(p->state == BLOCKED){
            p->state = READY;
            serial::log("[UNBLOCK] PID: %d", p->PID);
        }

        asm volatile("sti");
    }

    void exitProcess(){
        asm volatile("cli");

        if(getThisProcess() == initProc){
            serial::log("[ERROR] can't exit init proc");
            return;
        }

        getThisProcess()->state = KILLED;
        serial::log("[EXIT] PID: %d", getThisProcess()->PID);
        asm volatile("sti");
        _yield();
        while(true){
            asm volatile("hlt");
        }
    }

    void cleanProcess(process_t* p){
        if(p == nullptr){
            return;
        }

        if(p->state == KILLED){
            for(size_t i = 0; i < p->children.getLength(); ++i){
                process_t* childProcess = p->children[i];
                childProcess->state = KILLED;
                cleanProcess(childProcess);
            }

            for(size_t i = 0; i < p->threads.getLength(); ++i){
                p->threads.remove(p->threads.find(p->threads[i]));
                free(p->threads[i]->threadStack);
                free(p->threads[i]);
                threadsCounter--;
            }

            process_t* parentProcess = p->parent;
            if(parentProcess != nullptr){
                parentProcess->children.remove(parentProcess->children.find(p));
                if(parentProcess->children.getLength() == 0 && p->threads.getLength() == 0){
                    parentProcess->state = KILLED;
                    cleanProcess(parentProcess);
                }
            }

            free(p->processPagemap);
            free(p);
            processesCounter--;
        }
        else {
            for(size_t i = 0; i < p->threads.getLength(); ++i){
                if(p->threads[i]->state == KILLED){
                    p->threads.remove(p->threads.find(p->threads[i]));
                    free(p->threads[i]->threadStack);
                    free(p->threads[i]);
                    threadsCounter--;
                }
            }

            if(p->children.getLength() == 0 && p->threads.getLength() == 0){
                p->state = KILLED;
                cleanProcess(p);
            }
        }
    }

    void switchTask(registers_t* reg){
        if(!isInit){
            return;
        }

        schedLock.lock();
        timeSlice = DEFAULT_TIMESLICE;

        if(!getThisProcess() || !getThisThread()){
            for(size_t i = 0; i < processTable.getLength(); ++i){
                process_t* p = processTable[i];
                if(p->state != READY){
                    cleanProcess(p);
                    continue;
                }

                for(size_t j = 0; j < p->threads.getLength(); ++j){
                    thread_t* t = p->threads[j];
                    if(t->state != READY){
                        continue;
                    }

                    thisCPU->currentProcess = p;
                    thisCPU->currentThread = t;
                    timeSlice = getThisThread()->sliceOfTime;
                    subSuccess(t, reg);
                    return;
                }
            }
            subNoFree(getThisProcess(), getThisThread(), reg);
            return;
        }
        else {
            getThisThread()->reg = *reg;

            if(getThisThread()->state == RUNNING){
                getThisThread()->state = READY;
            }

            for(size_t i = getThisProcess()->threads.find(getThisThread()) + 1; i < getThisProcess()->threads.getLength(); ++i){
                thread_t* t = getThisProcess()->threads[i];

                if(getThisProcess()->state != READY){
                    break;
                }

                if(t->state != READY){
                    continue;
                }

                thisCPU->currentProcess = getThisProcess();
                thisCPU->currentThread = t;
                timeSlice = getThisThread()->sliceOfTime;
                subSuccess(getThisThread(), reg);
                return;
            }

            for(size_t i = 0; i < processTable.find(getThisProcess()) + 1; ++i){
                process_t* p = processTable[i];
                
                if(p->state != READY){
                    continue;
                }

                for(size_t j = 0; j < p->threads.getLength(); ++j){
                    thread_t* t = p->threads[i];
                    if(t->state != READY){
                        continue;
                    }

                    cleanProcess(getThisProcess());

                    thisCPU->currentProcess = p;
                    thisCPU->currentThread = t;
                    timeSlice = getThisThread()->sliceOfTime;
                    subSuccess(t, reg);
                    return;
                }
            }
        }

        subNoFree(getThisProcess(), getThisThread(), reg);
        return;
    }

    bool isIDTInit = false;

    void init(){
        printf("here 1\n");
        while(!isInit){
            asm volatile("hlt");
        }
        printf("here 2\n");
        if(apic::isInit){
            if(schedulerVector == 0){
                schedulerVector = idt::allocVector();
                if(!isIDTInit){
                    idt::registerInterruptHandler(schedulerVector, switchTask);
                    idt::idtSetDescriptor(schedulerVector, idt::int_table[schedulerVector], 0x8E, 1);
                    isIDTInit = true;
                }
            }

            apic::lapicPeriodic(schedulerVector);
        }
        else {
            if(!isIDTInit){
                idt::idtSetDescriptor(schedulerVector, idt::int_table[idt::IRQ0], 0x8E, 1);
                isIDTInit = true;
            }
        }
        while(true){
            asm volatile("hlt");
        }
    }   
}