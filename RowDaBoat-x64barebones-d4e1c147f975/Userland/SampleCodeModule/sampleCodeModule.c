// sampleCodeModule.c
#include <usrlib.h>
#include <shell.h>

extern void _hlt();
void dummyFuction();

int main(){

    int pid;
    char * argv[] = {"Shell"};
    pid = initializeProccess((int (*)(int, char**))startShell, 1, 1, argv, 0);
    changeProccessPriority(pid, 4);
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
