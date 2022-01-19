#pragma once 

#include <stdint.h>

namespace turbo::hpet {
	#define SECS(num) ((num) * 1000000)
	#define MSECS(num) ((num) * 10000)
	#define MISECS(num) ((num) * 10)

	struct HPET {
		uint64_t generalCapabilities;
		uint64_t Reserved;
		uint64_t generalConfiguration;
		uint64_t Reserved2;
		uint64_t generalInterruptStatus;
		uint64_t Reserved3;
		uint64_t Reserved4[24];
		uint64_t mainCounterValue;
		uint64_t Reserved5;
	};

	extern bool isInit;
	extern HPET* hpet;

	char* hour();

	uint64_t counter();

	void uSleep(uint64_t uSeconds);
	void mSleep(uint64_t mSeconds);
	void sleep(uint64_t seconds);

	void init();
}