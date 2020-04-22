// sampleCodeModule.c
#include <usrlib.h>
#include <shell.h>

extern void _hlt();
void dummyFuction();

int main(){

    int pid;

    pid = initializeProccess((int (*)(int, char**))startShell, "Shell", 1, 0, 0);
    changeProccessPriority(pid, 0);

    while(1){
        println("Estamos en sample");
        _hlt();
    }

    // startShell();


    // clearScreen();
    // setCursorPos(0, 0);
    // print("Sesion finalizada");
    return 0;
}

void dummyFuction(){
     while(1){
        println("I´m dummy");
        _hlt();
    }
}
