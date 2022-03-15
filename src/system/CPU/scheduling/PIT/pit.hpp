#pragma once

#include <stdint.h>

namespace turbo::pit {

	#define PIT_DEFAULT_FREQUENCE 100
	#define MS_TO_PIT(ms) (100/(ms))
	#define PIT_TO_MS(freq) (100 / (freq))
	
	extern bool isInit;
	extern bool isScheduling;
	extern uint64_t frequence;

	void sleep(uint64_t seconds);
	void mSleep(uint64_t mSeconds);
	
	uint64_t getTick();
	uint64_t getFrequence();

	void setFrequence(uint64_t freq = PIT_DEFAULT_FREQUENCE);

	void init(uint64_t freq = PIT_DEFAULT_FREQUENCE);
}

