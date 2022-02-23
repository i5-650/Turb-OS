#include <system/CPU/scheduling/ohMyTime/omtime.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <system/CPU/scheduling/PIT/pit.hpp>
#include <system/CPU/scheduling/RTC/rtc.hpp>
#include <drivers/display/serial/serial.hpp>

using namespace turbo;

namespace turbo::omtime {

	void sleep(uint64_t seconds){
		if(hpet::isInit){
			hpet::sleep(seconds);
		}
		else if(pit::isInit){
			pit::sleep(seconds);
		}
		else{
			rtc::sleep(seconds);
		}
	}

	void mSleep(uint64_t mSeconds){
		if(hpet::isInit){
			hpet::mSleep(mSeconds);
		}
		else if(pit::isInit){
			pit::mSleep(mSeconds);
		}
		else{
			rtc::sleep(mSeconds / 100);
		}
	}

	void uSleep(uint64_t uSeconds){
		if(hpet::isInit){
			hpet::uSleep(uSeconds);
		}
		else if(pit::isInit){
			pit::mSleep(US_TO_MSEC(uSeconds));
		}
		else{
			rtc::sleep(US_TO_SEC(uSeconds));
		}
	}
}
