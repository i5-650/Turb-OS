#pragma once

#include <system/CPU/GDT/gdt.hpp>

namespace turbo::smp {

	struct cpu_t {
		uint64_t cpuID;
	};
}