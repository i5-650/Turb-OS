#pragma once

#include <stdint.h>

namespace turbo::omtime{

	#define SEC_TO_US(num) ((num) * 1000000)
	#define MSEC_TO_US(num) ((num) * 10000)
	#define MICROSEC_TO_US(num) ((num) * 10)

	#define US_TO_SEC(num) ((num) / 1000000)
	#define US_TO_MSEC(num) ((num) / 10000)
	#define US_TO_MICROSEC(num) ((num) / 10)
	
	void sleep(uint64_t seconds);
	void mSleep(uint64_t mSeconds);
	void uSleep(uint64_t uSeconds);
}
