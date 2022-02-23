#pragma once

#include <stdint.h>

namespace turbo::rtc{

    void init();

    int bcptobin(int value);// convert binary decimal to binary

    uint64_t dateToSeconds(uint64_t day, uint64_t month, uint64_t year);

    uint64_t century();
    uint64_t year();
    uint64_t month();
    uint64_t day();
    uint64_t weekDay();
    uint64_t hour();
    uint64_t minute();
    uint64_t second();

    
    uint64_t time(); //return actual hour / minutes / seconds in seconds
    
    uint64_t epoch(); //return actual date in seconds

    void sleep(uint64_t seconds);

    char* getTime();
}