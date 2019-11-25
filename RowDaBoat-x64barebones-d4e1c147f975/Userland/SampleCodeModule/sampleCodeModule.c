// sampleCodeModule.c
#include <usrlib.h>
#include <shell.h>

int main(){
    startShell();

    clearScreen();
    setCursorPos(0, 0);
    print("Sesion finalizada");

    return 0;
}
