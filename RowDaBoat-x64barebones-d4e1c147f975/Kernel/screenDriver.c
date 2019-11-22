#include <screenDriver.h>

unsigned int cursorPosX = 0;
unsigned int cursorPosY = 0;
unsigned int screenWidth;
unsigned int screenHeight; 

void putchar(char c){
    putcharf(c, DEFAULT_FONT_COLOR, DEFAULT_BACKGROUND_COLOR );
}

void init_screen(){
    screenWidth = horPixelCount() / CHAR_WIDTH;
    screenHeight = verPixelCount() / CHAR_HEIGHT; 
} 

void setCursorPos(unsigned int x, unsigned int y){
    if(x < 0 || x >= screenWidth || y < 0 || y >= screenHeight)
        return;

    cursorPosX = x;
    cursorPosY = y;
}


void putcharf(char c, unsigned int font, unsigned int background ){
    if(cursorPosY>=screenHeight){
        scrollDownOnce();
    }
    drawChar(cursorPosX*CHAR_WIDTH, cursorPosY*CHAR_HEIGHT, c, font, background);
    cursorPosX++;
    if(cursorPosX >= screenWidth){
        cursorPosX=0;
        cursorPosY++;
    }
}

void printString( char * string){
    while(*string){
        putchar(*string);
        string++;
    }
}
void printStringf( char * string, unsigned int font, unsigned int background){
    while(*string){
        putcharf(*string, font, background);
        string++;
    }
}

void scrollDownOnce(){
    for(int j=0; j < verPixelCount() - CHAR_HEIGHT; j++){
        for(int i=0; i < horPixelCount(); i++){
            copyPixel(i, j+CHAR_HEIGHT, i, j);
        }
    }
    setCursorPos(0, screenHeight-1);
    for(int i=0 ; i<screenWidth ; i++){
        putchar(' ');
    }
    setCursorPos(0, screenHeight-1);
}

void clearScreen(){
    int x= cursorPosX;
    int y= cursorPosY;
    setCursorPos(0,0);
    for(int j=0; j<screenHeight*screenWidth; j++)
        putchar(' ');

    setCursorPos(x, y);
}