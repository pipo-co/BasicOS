#ifndef __USR_LIB_H__
#define __USR_LIB_H__
#include <stdint.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

extern uint64_t * getRegs();

uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

void print(char* string);

void printf(char* string, unsigned int fontColor, unsigned int backgroundcolor);

void putchar(char c);

void putcharf(char c, unsigned int fontColor, unsigned int backgroundcolor);

void println(char* string);

void printint(uint64_t value);

void clearScreen();

int getScreenWidth();

int getScreenHeight();

int strcmp(char * s1, char * s2);

int strlen(char * s);

int strtok(char * s, char delim, char * array[], int arraySize);

//SysCalls
extern void sysWrite(char * string, unsigned int font, unsigned int background);
extern int getTicksElapsed();
extern void drawPixel(unsigned int x, unsigned int y, int color);
extern char getChar();
extern void setCursorPos(unsigned int x, unsigned int y);
extern int verticalPixelCount();
extern int horizontalPixelCount();
extern uint8_t getTime(uint64_t timeDescriptor);

#endif