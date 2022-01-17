#include <drivers/display/terminal/terminal.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/printf.h>
#include <lib/string.hpp>
#include <drivers/display/serial/serial.hpp>
#include <apps/turboShell.hpp>
#include <system/ACPI/acpi.hpp>

namespace turbo::shell{

    void parse(char* cmd){
        switch(hash(cmd)){
            case hash("turbo"):
                printf("https://www.instagram.com/http.doggyboys.fr/\n");
                break;
            case hash("help"):
                printf("All the listed command are not totally implement !\n");
                printf("-turbo --Display the link to access to the Instagram account of the DoggyBoys !\n");
                printf("-help --Display all the implement commands and future implemented command !\n");
                break;
        }
    }

    void run(){
        turbo::serial::log("Starting the TurboShell ! \n");
        while (true)
        {
            printf("root@turboShell: ");
            char *command = turbo::keyboard::getLine();
            parse(command);
        }
        
    }
}