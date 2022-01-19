#include <rtc.hpp>
#include <system/ACPI/acpi.hpp>
#include <lib/portIO.hpp>

using namespace rtc,acpi;

#define CMOS_IN 0x70;
#define CMOS_OUT 0x71;

int bcptobin(int value){
    return (value >> 4)*10 + (value&15);
}

uint64_t dateToJulianDateDay(uint64_t days, uint64_t months, uint64_t years){ // convert a day to the number of day passed in the Julian calender
    return (1461 * (years + 4800 + (months - 14) / 12)) / 4 + (367 * (months - 2 - 12 * ((months - 14) / 12))) / 12 - (3 * ((years + 4900 + (months - 14) / 12) / 100)) / 4 + days - 32075
}

uint64_t century(){
    if(fadthdr && fadthdr->century == 0) return 20;

    outb(CMOS_IN,0x32); 
    return bcptobin(inb(CMOS_OUT));
}

uint64_t year(){
    outb(CMOS_IN,0x09);
    return bcptobin(inb(CMOS_OUT));
}

uint64_t month(){
    outb(CMOS_IN,0x08);
    return bcptobin(inb(CMOS_OUT))
}

uint64_t day(){
    outb(CMOS_IN,0x07);
    return bcptobin(inb(CMOS_OUT))
}

uint64_t weekDay(){
    outb(CMOS_IN,0x06);
    return bcptobin(inb(CMOS_OUT))
}

uint64_t hours(){
    outb(CMOS_IN,0x04);
    return bcptobin(inb(CMOS_OUT))
}

uint64_t minute(){
    outb(CMOS_IN,0x02);
    return bcptobin(inb(CMOS_OUT))
}

uint64_t second(){
    outb(CMOS_IN,0x00);
    return bcptobin(inb(CMOS_OUT))
}

uint64_t time(){
    return hour()*3600 + minute()*60 + second();
}

uint64_t epoch(){
    uint64_t seconds=second(), minutes=minute(), hours=hours(), days=day(), months=month(), years=year(),centuries=century();

    uint64_t dateCurrent = dateToJulianDateDay(days, months, centuries*100+years);
    uint64_t date1970 = dateToJulianDateDay(1,1,1970);
    uint64_t diff = dateCurrent - date1970;

    return (diff*(60*60*24)) + hours*3600 + minutes*60 + seconds;
}


