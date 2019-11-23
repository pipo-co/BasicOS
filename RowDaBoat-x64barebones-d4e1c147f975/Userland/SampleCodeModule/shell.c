#include <shell.h>
#include <usrlib.h>

void startShell(){
    clearScreen();
    setCursorPos(0,getScreenHeight() - 1);
    print("Bienvenidos");
}
