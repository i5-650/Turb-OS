#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <system/CPU/scheduling/ohMyTime/omtime.hpp>
#include <system/CPU/scheduling/PIT/pit.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/CPU/SMP/smp.hpp>
#include <kernel/kernel.hpp>
#include <lib/string.hpp>
#include <drivers/display/serial/serial.hpp>
#include <lib/TurboVector/TurboVector.hpp>
#include <system/memory/heap/heap.hpp>

namespace turbo::scheduler {

    bool isInit = false;
    static uint64_t next_pid = 1;
    static uint8_t sched_vector = 0;

    TurboVector<process_t*> proc_table;
    process_t *initproc = nullptr;

    size_t proc_count = 0;
    size_t thread_count = 0;

    DEFINE_LOCK(thread_lock);
    DEFINE_LOCK(sched_lock);
    DEFINE_LOCK(proc_lock);

    thread_t *allocThread(uint64_t addr, uint64_t args){
        thread_lock.lock();
        thread_t *thread = new thread_t;

        thread->state = INITIAL_STATE;
        thread->thread_stack = (uint8_t*)malloc(STACK_SIZE);

        thread->thread_regs.rflags = 0x202;
        thread->thread_regs.cs = 0x28;
        thread->thread_regs.ss = 0x30;
        thread->thread_regs.rip = addr;
        thread->thread_regs.rdi = (uint64_t)args;
        thread->thread_regs.rsp = ((uint64_t)thread->thread_stack) + STACK_SIZE;

        thread->parent_proc = nullptr;
        thread_lock.unlock();

        return thread;
    }

    thread_t *createThread(uint64_t addr, uint64_t args, process_t *parent){
        thread_t *thread = allocThread(addr, args);

        if(parent){
            thread->TID = parent->nextTID++;
            thread->parent_proc = parent;
            parent->threadsVec.push_back(thread);
        }

        thread_count++;
        thread_lock.lock();
        thread->state = READY;
        thread_lock.unlock();

        return thread;
    }

    void idle(){
        while(true){
            asm volatile ("hlt");
        }
    }

    process_t *allocProc(const char *name, uint64_t addr, uint64_t args){
        process_t *proc = new process_t;

        proc_lock.lock();
        strncpy(proc->name, name, (strlen(name) < 128) ? strlen(name) : 128);
        proc->PID = next_pid++;
        proc->state = INITIAL_STATE;
        proc->pagemap = vMemory::newPagemap();
        proc->parent = nullptr;

        if(addr){
            createThread(addr, args, proc);
        }

        if(!isInit){
            initproc = proc;
            isInit = true;
        }

        proc_lock.unlock();

        return proc;
    }

    process_t *createProc(const char *name, uint64_t addr, uint64_t args){
        process_t *proc = allocProc(name, addr, args);
        proc_table.push_back(proc);
        proc_count++;
        proc_lock.lock();
        proc->state = READY;
        proc_lock.unlock();

        return proc;
    }

    thread_t *this_thread(){
        asm volatile ("cli");
        thread_t *thread = thisCPU->currentThread;
        asm volatile ("sti");
        return thread;
    }

    process_t *this_proc(){
        asm volatile ("cli");
        process_t *proc = thisCPU->currentProcess;
        asm volatile ("sti");
        return proc;
    }

    void _yield(uint64_t ms){
        if(apic::isInit){
            apic::lapicOneShot(sched_vector, ms);
        }
        else{
            pit::setFrequence(MS_TO_PIT(ms));
        }
    }

    void blockThread(){
        asm volatile ("cli");
        if(this_thread()->state == READY || this_thread()->state == RUNNING){
            this_thread()->state = BLOCKED;
            serial::log("Blocking thread with TID: %d and PID: %d", this_thread()->TID, this_proc()->PID);
        }

        asm volatile ("sti");
        _yield();
    }

    void blockThread(thread_t *thread){
        asm volatile ("cli");

        if(this_thread()->state == READY || this_thread()->state == RUNNING){
            thread->state = BLOCKED;
            serial::log("Blocking thread with TID: %d and PID: %d", thread->TID, thread->parent_proc->PID);
        }

        asm volatile ("sti");
    }

    void blockProc(){
        asm volatile ("cli");
        if(this_proc() == initproc){
            serial::log("Can not block init process!");
            asm volatile ("sti");
            return;
        }

        if(this_proc()->state == READY || this_proc()->state == RUNNING){
            this_proc()->state = BLOCKED;
            serial::log("Blocking process with PID: %d", this_proc()->PID);
        }
        asm volatile ("sti");
        _yield();
    }

    void blockProc(process_t *proc){
        asm volatile ("cli");

        if(this_proc()->state == READY || this_proc()->state == RUNNING){
            proc->state = BLOCKED;
            serial::log("Blocking process with PID: %d", proc->PID);
        }

        asm volatile ("sti");
    }

    void thread_unblock(thread_t *thread){
        asm volatile ("cli");

        if(thread->state == BLOCKED){
            thread->state = READY;
            serial::log("Unblocking thread with TID: %d and PID: %d", thread->TID, thread->parent_proc->PID);
        }

        asm volatile ("sti");
    }

    void proc_unblock(process_t *proc){
        asm volatile ("cli");

        if(proc->state == BLOCKED){
            proc->state = READY;
            serial::log("Unblocking process with PID: %d", proc->PID);
        }

        asm volatile ("sti");
    }

    void exitThread(){
        asm volatile ("cli");
        if(this_proc() == initproc && this_proc()->threadsVec.size() == 1 && this_proc()->children.size() == 0){
            serial::log("Can not kill init process!");
            return;
        }
        this_thread()->state = KILLED;
        serial::log("Exiting thread with TID: %d and PID: %d", this_thread()->TID, this_proc()->PID);
        asm volatile ("sti");
        _yield();

        while(true){
            asm volatile ("hlt");
        }
    }

    void exitProc(){
        asm volatile ("cli");
        if(this_proc() == initproc){
            serial::log("Can not kill init process!");
            return;
        }

        this_proc()->state = KILLED;
        serial::log("Exiting process with PID: %d", this_proc()->PID);
        asm volatile ("sti");
        _yield();

        while(true){
            asm volatile ("hlt");
        }
    }

    void clean_proc(process_t *proc){
        if(proc == nullptr){
            return;
        }

        if(proc->state == KILLED){

            for(size_t i = 0; i < proc->children.size(); i++){
                process_t *childproc = proc->children[i];
                childproc->state = KILLED;
                clean_proc(childproc);
            }

            for(size_t i = 0; i < proc->threadsVec.size(); i++){
                thread_t *thread = proc->threadsVec[i];
                proc->threadsVec.remove(proc->threadsVec.find(thread));
                free(thread->thread_stack);
                free(thread);
                thread_count--;
            }

            process_t *parentproc = proc->parent;

            if(parentproc != nullptr){
                parentproc->children.remove(parentproc->children.find(proc));
                
                if(parentproc->children.size() == 0 && proc->threadsVec.size() == 0){
                    parentproc->state = KILLED;
                    clean_proc(parentproc);
                }
            }

            free(proc->pagemap);
            free(proc);
            proc_count--;
        }
        else{
            for(size_t i = 0; i < proc->threadsVec.size(); i++){
                thread_t *thread = proc->threadsVec[i];
                if(thread->state == KILLED){
                    proc->threadsVec.remove(proc->threadsVec.find(thread));
                    free(thread->thread_stack);
                    free(thread);
                    thread_count--;
                }
            }

            if(proc->children.size() == 0 && proc->threadsVec.size() == 0){
                proc->state = KILLED;
                clean_proc(proc);
            }
        }
    }

    void switchTask(registers_t *regs){
        if(!isInit){
            return;
        }

        sched_lock.lock();
        uint64_t timeslice = DEFAULT_TIMESLICE;

        if(!this_proc() || !this_thread()){
            for(size_t i = 0; i < proc_table.size(); i++){
                process_t *proc = proc_table[i];

                if(proc->state != READY){
                    clean_proc(proc);
                    continue;
                }

                for(size_t t = 0; t < proc->threadsVec.size(); t++){
                    thread_t *thread = proc->threadsVec[t];
                    
                    if(thread->state != READY){
                        continue;
                    }

                    thisCPU->currentProcess = proc;
                    thisCPU->currentThread = thread;
                    timeslice = this_thread()->sliceOfTime;
                    goto success;
                }
            }
            goto nofree;
        }
        else{
            this_thread()->thread_regs = *regs;

            if(this_thread()->state == RUNNING){
                this_thread()->state = READY;
            }

            for (size_t t = this_proc()->threadsVec.find(this_thread()) + 1; t < this_proc()->threadsVec.size(); t++){
                thread_t *thread = this_proc()->threadsVec[t];

                if(this_proc()->state != READY){
                    break;
                }

                if(thread->state != READY){
                    continue;
                }

                thisCPU->currentProcess = this_proc();
                thisCPU->currentThread = thread;
                timeslice = this_thread()->sliceOfTime;
                goto success;
            }
            for (size_t p = proc_table.find(this_proc()) + 1; p < proc_table.size(); p++){
                process_t *proc = proc_table[p];
                if(proc->state != READY){
                    continue;
                }

                for (size_t t = 0; t < proc->threadsVec.size(); t++){
                    thread_t *thread = proc->threadsVec[t];
                    if(thread->state != READY){
                        continue;
                    }

                    clean_proc(this_proc());

                    thisCPU->currentProcess = proc;
                    thisCPU->currentThread = thread;
                    timeslice = this_thread()->sliceOfTime;
                    goto success;
                }
            }
            for (size_t p = 0; p < proc_table.find(this_proc()) + 1; p++){
                process_t *proc = proc_table[p];
                if(proc->state != READY){
                    continue;
                }

                for(size_t t = 0; t < proc->threadsVec.size(); t++){
                    thread_t *thread = proc->threadsVec[t];
                    if(thread->state != READY){
                        continue;
                    }

                    clean_proc(this_proc());

                    thisCPU->currentProcess = proc;
                    thisCPU->currentThread = thread;
                    timeslice = this_thread()->sliceOfTime;
                    goto success;
                }
            }
        }
        goto nofree;

        success:;
        this_thread()->state = RUNNING;
        *regs = this_thread()->thread_regs;
        vMemory::switchPagemap(this_proc()->pagemap);

        //serial::log("Running process[%d]->thread[%d] on CPU core %zu", this_proc()->pid - 1, this_thread()->tid - 1, thisCPU->lapicID);

        sched_lock.unlock();
        _yield(timeslice);
        return;

        nofree:
        clean_proc(this_proc());

        if (thisCPU->idleP == nullptr){
            thisCPU->idleP = allocProc("Idle", reinterpret_cast<uint64_t>(idle), 0);
            thread_count--;
        }

        thisCPU->currentProcess = thisCPU->idleP;
        thisCPU->currentThread = thisCPU->idleP->threadsVec[0];
        timeslice = this_thread()->sliceOfTime;

        this_thread()->state = RUNNING;
        *regs = this_thread()->thread_regs;
        vMemory::switchPagemap(this_proc()->pagemap);

        //serial::log("Running Idle process on CPU core %zu", thisCPU->lapicID);

        sched_lock.unlock();
        _yield();
    }

    void init(){
        if (apic::isInit){
            if (sched_vector == 0){
                sched_vector = idt::allocVector();
                idt::registerInterruptHandler(sched_vector, switchTask);
                idt::idtSetDescriptor(sched_vector, idt::int_table[sched_vector], 0x8E, 1);
            }
            apic::lapicPeriodic(sched_vector);
        }
        else{
            idt::idtSetDescriptor(sched_vector, idt::int_table[idt::IRQ0], 0x8E, 1);
            pit::isScheduling = true;
        }
    }
}