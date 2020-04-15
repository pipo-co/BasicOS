// sampleCodeModule.c
#include <usrlib.h>
#include <shell.h>

int main(){

    int * v1 = malloc2(50000);
    int * v2 = malloc2(50000);
    int * v3 = malloc2(50000);
    int * v4 = malloc2(50000);
    int * v5 = malloc2(50000);
    int * v6 = malloc2(1);
    free2(v3);
    
    startShell();


    // clearScreen();
    // setCursorPos(0, 0);
    // print("Sesion finalizada");
    return 0;
}
