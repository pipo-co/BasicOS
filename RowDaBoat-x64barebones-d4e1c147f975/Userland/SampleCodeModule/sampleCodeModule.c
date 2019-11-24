/* sampleCodeModule.c */
#include <usrlib.h>
#include <shell.h>
#include <arkanoid.h>

int main(){
    // for (int i = 0; i < 1500; i++)
    //     for (int j = 0; j < 1500; j++)
    //         drawPixel(i,j,(i*j % 50) + 0x00AA00);
    // char aux[5];
    // int currentTick, lastTick = -1;
    // uintToBase(horizontalPixelCount(), aux, 10);
    // sysWrite(aux,0xAAFF12, 0x111111);
    // uintToBase(verticalPixelCount(), aux, 10);
    // sysWrite(aux,0xAAFF12, 0x111111);
    // char c;
    // char str[] = {0, 0};
    // int x = 0;
    // while(1){
    //     currentTick = getTicksElapsed();
    //     c = getChar();
    //     if(currentTick != lastTick){
    //         setCursorPos(currentTick % (horizontalPixelCount() / CHAR_WIDTH), currentTick % (verticalPixelCount() / CHAR_HEIGHT));
    //         uintToBase(getTicksElapsed(),aux,10);
    //         sysWrite(aux,0xAAFF12, 0x111111);
    //         sysWrite(" ",0xAAFF12, 0x111111);
    //         lastTick = currentTick;
    //     }
    //     if(c){
    //         str[0] = c;
    //         setCursorPos(x, verticalPixelCount() / CHAR_HEIGHT - 1);
    //         sysWrite(str, 0xFFFFFF, 0);
    //         x++;
    //         x = x % (horizontalPixelCount() / CHAR_WIDTH);
    //     }
    // }

    // while(1);
    startArkanoid();
    return 0;
}