// sampleCodeModule.c
#include <usrlib.h>
#include <shell.h>

extern void _hlt();

int main(){

    initializeProccess((int (*)(int, char**))startShell, "Shell", 1, 0, 0);


    while(1){
        println("Estamos en sample");
        _hlt();
    }

    // startShell();


    // clearScreen();
    // setCursorPos(0, 0);
    // print("Sesion finalizada");
    // return 0;
}
