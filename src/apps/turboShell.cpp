#include <drivers/display/terminal/terminal.hpp>
#include <system/CPU/scheduling/HPET/hpet.hpp>
#include <drivers/devices/keyboard.hpp>
#include <drivers/display/terminal/printf.h>
#include <lib/string.hpp>
#include <drivers/display/serial/serial.hpp>
#include <apps/turboShell.hpp>
#include <system/ACPI/acpi.hpp>
#include <stddef.h>
#include <lib/panic.hpp>
#include <drivers/fs/vfs/turboVFS.hpp>
#include <system/CPU/scheduling/scheduler/scheduler.hpp>


namespace turbo::shell{
    
    turbo::vfs::tfs_node_t *currentPath;

    turbo::vfs::tfs_node_t *currentPath;

    void parse(char* cmd,char *arg){
        switch(hash(cmd)){
            case hash("turbo"):
                printf("2FAST4U\n");
                break;

            case hash("help"):
                printf("All the listed command are not totally implement !\n");
                printf("-turbo --Display the link to access to the Instagram account of the DoggyBoys !\n");
                printf("-help --Display all the implement commands and future implemented command !\n");
                break;

            case hash(""):
                break;
                
            case hash("panic"):
                PANIC("WHAT IS THE PPROBLEM ?");
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

        while(true){
            if(!currentPath){
                currentPath = turbo::scheduler::getThisProcess()->current_dir;
                currentPath->flags = turbo::vfs::TFS_FOLDER;
            }
            printf("root@turboShell: ");
            char *command = turbo::keyboard::getLine();
            char tmp[10] = "\0";

            for(size_t i = 0; i<strlen(command);i++){
                if(command[i] != ' ' && command[i] != '0'){
                   char c[2] = "\0";
                    c[0] = command[i];
                    strcat(tmp,c); 
                }else{
                    break;
                }
                char *arguments = strrm(command, tmp);
                arguments = strrm(arguments," ");

                parse(tmp,arguments);
            }
        }
    }
}