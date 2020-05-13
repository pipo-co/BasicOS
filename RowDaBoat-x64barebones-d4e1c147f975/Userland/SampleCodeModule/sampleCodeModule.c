// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <usrlib.h>
#include <shell.h>

extern void _hlt();
void dummyFuction();

int main(){

    char * argv[] = {"Shell"};
    initializeProccess((void (*)(int, char**))startShell, 1, 1, argv, 0);
    // startShell();


    clearScreen();
    setCursorPos(0, 0);
    print("Sesion finalizada");
    return 0;
}

void dummyFuction(){
     while(1){
        println("I´m dummy");
        _hlt();
    }
}
