#ifndef __SCREENDRIVER_H__
#define __SCREENDRIVER_H__

#include <videoDriver.h>

#define DEFAULT_BACKGROUND_COLOR 0X000000
#define DEFAULT_FONT_COLOR 0xFFFFFF

void init_screen();

void putchar(char c);

void setCursorPos(unsigned int x, unsigned int y);

void putcharf(char c, unsigned int font, unsigned int background );

void printString( char * string);

void printStringf( char * string, unsigned int font, unsigned int background);

void scrollDownOnce();

void clearScreen();

#endif