#ifndef __USR_LIB_H__
#define __USR_LIB_H__
#include <stdint.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16


uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

extern void sysWrite(char * string, unsigned int font, unsigned int background);
extern int getTicksElapsed();
extern void drawPixel(unsigned int x, unsigned int y, int color);
extern char getChar();
extern void setCursorPos(unsigned int x, unsigned int y);
extern int verticalPixelCount();
extern int horizontalPixelCount();

#endif