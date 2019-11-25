//screenDriver.h
#ifndef __SCREENDRIVER_H__
#define __SCREENDRIVER_H__

#include <videoDriver.h>

#define DEFAULT_BACKGROUND_COLOR 0X000000
#define DEFAULT_FONT_COLOR 0xFFFFFF

//Realiza la iniciacion del video driver y define el tamaño de la matriz de caracteres.
void init_screen();

void putchar(char c);

//Funcion asginada a la sysCall 4
//Permite asignar la posicion de inicio de la proxima impresion a pantalla, siempre y cuando
// la posicion se encuentre entre los limites de la pantalla
void setCursorPos(unsigned int x, unsigned int y);

//Tanto putcharf como printStringf permiten imprmir a pantalla en la posicion del cursor 
// utilizando un color (HEX) seleccionable para la letra y el fondo.
void putcharf(char c, unsigned int font, unsigned int background );

void printString( char * string);

void printStringf( char * string, unsigned int font, unsigned int background);

//Deja la pantalla vacia y color de fondo default (NEGRO)
void clearScreen();

//Print pero ahora con enter DoU
void println(char * string);

#endif