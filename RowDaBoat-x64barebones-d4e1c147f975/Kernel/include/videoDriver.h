#ifndef __VIDEODRIVER_H__
#define __VIDEODRIVER_H__

#include <stdint.h>
#include <font.h>

void init_VM_Driver();

void drawPixel(unsigned int x, unsigned int y, int color);

void drawChar(int x, int y, char character, int fontColor, int backgroundColor);

int horPixelCount();

int verPixelCount();

void copyPixel(unsigned int xFrom, unsigned int yFrom, unsigned int xTo, unsigned int yTo);

#endif