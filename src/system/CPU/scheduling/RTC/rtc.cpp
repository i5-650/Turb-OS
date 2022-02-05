#include <system/CPU/scheduling/RTC/rtc.hpp>
#include <system/ACPI/acpi.hpp>
#include <lib/portIO.hpp>
#include <drivers/display/terminal/printf.h>
#include <drivers/display/serial/serial.hpp>

using namespace turbo::acpi;

namespace turbo::rtc{
    #define CMOS_IN 0x70
    #define CMOS_OUT 0x71

    void init(){
        turbo::serial::log("Lancement init RTC... OK\n");
        printf("Lancement init RTC\n");
        getTime();
        turbo::serial::log("Fin init RTC\n");
    }


    int bcptobin(int value){
        return (value >> 4)*10 + (value&15);
    }

    uint64_t dateToJulianDateDay(uint64_t days, uint64_t months, uint64_t years){ // convert a day to the number of day passed in the Julian calender
        return (1461 * (years + 4800 + (months - 14) / 12)) / 4 + (367 * (months - 2 - 12 * ((months - 14) / 12))) / 12 - (3 * ((years + 4900 + (months - 14) / 12) / 100)) / 4 + days - 32075;
    }

    uint64_t century(){
        if(fadthdr && fadthdr->Century == 0) return 20;

        outb(CMOS_IN,0x32); 
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t year(){
        outb(CMOS_IN,0x09);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t month(){
        outb(CMOS_IN,0x08);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t day(){
        outb(CMOS_IN,0x07);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t weekDay(){
        outb(CMOS_IN,0x06);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t hour(){
        outb(CMOS_IN,0x04);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t minute(){
        outb(CMOS_IN,0x02);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t second(){
        outb(CMOS_IN,0x00);
        return bcptobin(inb(CMOS_OUT));
    }

    uint64_t time(){
        return hour()*3600 + minute()*60 + second();
    }

    uint64_t epoch(){
        uint64_t s=second(), m=minute(), h=hour(), d=day(), mo=month(), y=year(),c=century();

        uint64_t dateCurrent = dateToJulianDateDay(d, mo, c*100+y);
        uint64_t date1970 = dateToJulianDateDay(1,1,1970);
        uint64_t diff = dateCurrent - date1970;

        return (diff*(60*60*24)) + h*3600 + m*60 + s;
    }

    void sleep(uint64_t seconds){
        uint64_t timeWanted = time() + seconds;
        while(time()!=timeWanted);
    }

    void getTime(){
        turbo::serial::log("%.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld", century()*100 + year(), month(), day(), hour(), minute(), second());
        printf("%4ld/",century()*100+year());
        (month()<10) ? printf("0%ld/",month()) : printf("%2ld/",month());
        (day()<10) ? printf("0%ld - ",day()) : printf("%2ld - ",day());
        (hour()<10) ? printf("0%ld:",hour()) : printf("%2ld:",hour());
        (minute()<10) ? printf("0%ld:",minute()) : printf("%2ld:",minute());
        (second()<10) ? printf("0%ld\n",second()) : printf("%2ld\n",second());
    }

}


