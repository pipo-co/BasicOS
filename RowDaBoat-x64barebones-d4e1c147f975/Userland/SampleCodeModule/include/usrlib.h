#ifndef __USR_LIB_H__
#define __USR_LIB_H__
#include <stdint.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16


uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

void print(const char* string);

void printf(const char* string, unsigned int fontColor, unsigned int backgroundcolor);

void putchar(char c);

void putcharf(char c, unsigned int fontColor, unsigned int backgroundcolor);

void println(const char* string);

void clearScreen();

int getScreenWidth();

int getScreenHeight();

//SysCalls
extern void sysWrite(char * string, unsigned int font, unsigned int background);
extern int getTicksElapsed();
extern void drawPixel(unsigned int x, unsigned int y, int color);
extern char getChar();
extern void setCursorPos(unsigned int x, unsigned int y);
extern int verticalPixelCount();
extern int horizontalPixelCount();

#endif