#include <screenDriver.h>

unsigned int cursorPosX = 0;
unsigned int cursorPosY = 0;
unsigned int screenWidth;
unsigned int screenHeight;

static int parseSpecialCharacter(char c, unsigned int background);
static void backspace(unsigned int background);
static void enter(unsigned int background);
static void tab();
static void scrollDownOnce(unsigned int background);


void putchar(char c){
    putcharf(c, DEFAULT_FONT_COLOR, DEFAULT_BACKGROUND_COLOR );
}

void init_screen(){
    init_VM_Driver();
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
    if(parseSpecialCharacter(c, background))
        return;

    if(cursorPosY>=screenHeight)
        scrollDownOnce(background);

    drawChar(cursorPosX*CHAR_WIDTH, cursorPosY*CHAR_HEIGHT, c, font, background);

    cursorPosX++;
    if(cursorPosX >= screenWidth){
        cursorPosX = 0;
        cursorPosY++;
    }
}

//Se encarga de interpretar caractere de movimiento de cursor. Realizar un salto de 
// linea, borrar y realizar una tabulacion.
static int parseSpecialCharacter(char c, unsigned int background){
    switch(c){
        case '\n':
            enter(background);
            break;

        case '\b':
            backspace(background);
            break;

        case '\t':
            tab();
            break;

        default:
            return 0;
    }
    return 1;
}

void printString( char * string){
    while(*string){
        putchar(*string);
        string++;
    }
}

void println(char * string){
    printString(string);
    putchar('\n');
}

void printStringf( char * string, unsigned int font, unsigned int background){
    while(*string){
        putcharf(*string, font, background);
        string++;
    }
}

//Cuando se esta escribiendo en la ultima linea de la pantalla y se quiere pasar a la proxima 
// (fuera de la pantalla) sube todo lo impreso en pantalla y genera una nueva linea 
// para continuar escribiendo.

static void scrollDownOnce(unsigned int background){
    for(int j = 0; j < verPixelCount() - CHAR_HEIGHT; j++){
        for(int i = 0; i < horPixelCount(); i++){
            copyPixel(i, j+CHAR_HEIGHT, i, j);
        }
    }
    setCursorPos(0, screenHeight - 1);
    for(int i = 0 ; i < screenWidth; i++){
        putcharf(' ', 0, background);
    }
    setCursorPos(0, screenHeight-1);
}

void clearScreen(){
    int x = cursorPosX;
    int y = cursorPosY;
    setCursorPos(0,0);
    for(int j = 0; j < screenHeight*screenWidth; j++)
        putchar(' ');

    setCursorPos(x, y);
}

static void enter(unsigned int background){
    if(cursorPosY + 1 >= screenHeight)
        scrollDownOnce(background);
    else
        setCursorPos(0,cursorPosY + 1);
}

static void backspace(unsigned int background){
    if(cursorPosX == 0 && cursorPosY == 0)
            return;

    if(cursorPosX == 0)
        drawChar(cursorPosX*CHAR_WIDTH, cursorPosY*CHAR_HEIGHT, ' ', 0, background);
    else{
        drawChar((cursorPosX-1)*CHAR_WIDTH, cursorPosY*CHAR_HEIGHT, ' ', 0, background);
        setCursorPos(cursorPosX-1, cursorPosY);
    }
}

static void tab(){
    if(cursorPosX + 4 >= screenWidth)
        setCursorPos(screenWidth-1, cursorPosY);
    else
        setCursorPos(cursorPosX+4, cursorPosY);
}