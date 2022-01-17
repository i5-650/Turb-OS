#include <drivers/display/terminal/terminal.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/printf.h>
#include <lib/string.hpp>
#include <drivers/display/serial/serial.hpp>
#include <apps/turboShell.hpp>
#include <system/ACPI/acpi.hpp>
#include <stddef.h>


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

            case hash(""):
                break;

            default:
                printf("NEVER GONNA GIVE YOU UP, NEVER GONNA LET YOU DOWN !\n");
                printf("Unknown command\n");
                break;
        }
    }

    void run(){
        turbo::serial::log("Starting the TurboShell ! \n");

        printf("Press enter to begin...");
        turbo::keyboard::getLine();
        parse(nullptr);
        
        while (true){
            printf("root@turboShell: ");
            char *command = turbo::keyboard::getLine();
            parse(command);
        }
        
    }
}