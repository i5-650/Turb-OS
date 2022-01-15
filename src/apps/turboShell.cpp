#include <drivers/display/terminal/terminal.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/printf.h>
#include <lib/string.hpp>
#include <drivers/display/serial/serial.hpp>


namespace src::apps::turboShell{

    void parse(char* cmd,char* arg){
        switch(hash(cmd)){
            case hash("turbo"):
                printf("https://www.instagram.com/http.doggyboys.fr/");
                break;
            case(hash("help")):
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
            char cmd[15] = "\0";

            for(size_t i= 0;i<strlen(command);i++){
                if(command[i] != ' ' && command[i] != '\0'){
                    char c[2] = '\0';
                    c[0] = command[i];
                    strcat(cmd,c);
                }
                else break;
            }
            char *arg= strrm(command,cmd);
            arg=strrm(arg," ");

            parse(cmd,arg);
        }
        
    }
}