// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <IORouter.h>
#include <scheduler.h>
#include <keyboardDriver.h>
#include <screenDriver.h>
#include <pipe.h>

void routePrintStringf(char * string, unsigned int font, unsigned int background){

    uint16_t fd = getRunningProcessStdOut();

    if(fd == 0)
        printStringf(string, font, background);
    else
        writeStringPipe(fd, string);
}

char getchar(){
    uint16_t fd = getRunningProcessStdIn();

    if(fd == 0){
        if(amIFg())
            return getKey();
        else
            return 0; //Si alguien que no es foreground hace un getchar al teclado devuelve 0
    }

    return readPipe(fd);
}