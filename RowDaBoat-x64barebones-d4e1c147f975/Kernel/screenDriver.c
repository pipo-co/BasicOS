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
    if(cursorPosY>=screenHeight)
        scrollDownOnce();
    if(parseSpecialCharacter(c, font, background)){
        drawChar(cursorPosX*CHAR_WIDTH, cursorPosY*CHAR_HEIGHT, c, font, background);
    cursorPosX++;
    if(cursorPosX >= screenWidth){
        cursorPosX=0;
        cursorPosY++;
    }
    }
}
int parseSpecialCharacter(char c, unsigned int font, unsigned int background){
    if(c=='\n'){
        if(cursorPosY+1>=screenHeight)
            scrollDownOnce();
        else
            setCursorPos(0,cursorPosY + 1);
        return 0;
    }
    else if( c=='\b'){
        if(cursorPosX==0 && cursorPosY==0)
            return 0;
        if(cursorPosX == 0){
            drawChar((screenWidth-1)*CHAR_WIDTH, (cursorPosY-1)*CHAR_HEIGHT, ' ', font, background);
            setCursorPos(screenWidth-1, cursorPosY-1);
        }else{
            drawChar((cursorPosX-1)*CHAR_WIDTH, cursorPosY*CHAR_HEIGHT, ' ', font, background);
            setCursorPos(cursorPosX-1, cursorPosY);
        }
        return 0;
    }
    else if( c=='\t'){
        if(cursorPosX +4 >=screenWidth)
            setCursorPos(screenWidth-1, cursorPosY );
        else
        {
            setCursorPos(cursorPosX+4, cursorPosY);
        }
        return 0;
    }
    else
        return 1;
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