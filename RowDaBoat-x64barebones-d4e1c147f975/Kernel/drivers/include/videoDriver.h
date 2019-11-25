//videoDriver.h
//Archivo provisto por la catedra, con algunas modificaciones
#ifndef __VIDEODRIVER_H__
#define __VIDEODRIVER_H__

#include <stdint.h>
#include <font.h>

void init_VM_Driver();

//Funcion asociada a la sysCall 1, permite colorear un pixel especifico.
void drawPixel(unsigned int x, unsigned int y, int color);

void drawChar(int x, int y, char character, int fontColor, int backgroundColor);

//Funcion asociada a la sysCall 6. Getter del parametro horizontalPixelCount.
int horPixelCount();
//Funcion asociada a la sysCall 5. Getter del parametro verticalPixelCount.
int verPixelCount();

//Funcion agregada. Permite copiar el color de un pixel a otro. Utilizada 
// principalmente para el scrollDownOnce
void copyPixel(unsigned int xFrom, unsigned int yFrom, unsigned int xTo, unsigned int yTo);

#endif