#include <system/CPU/scheduling/scheduler/scheduler.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <system/CPU/APIC/apic.hpp>
#include <system/CPU/IDT/idt.hpp>
#include <system/CPU/SMP/smp.hpp>
#include <kernel/kernel.hpp>
#include <drivers/display/serial/serial.hpp>
#include <lib/string.hpp>

using namespace turbo;

namespace turbo::scheduler {
    bool isInit = false;


}