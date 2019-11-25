//userlib.h
#ifndef __USR_LIB_H__
#define __USR_LIB_H__
#include <stdint.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16

//Valores utilizados para llamar a la sysCall 7
enum time{HOURS = 4, MINUTES = 2, SECONDS = 0};

//Funcion sacada de naiveConsole.
uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

void print(char* string);

void printf(char* string, unsigned int fontColor, unsigned int backgroundcolor);

void putchar(char c);

void putcharf(char c, unsigned int fontColor, unsigned int backgroundcolor);

void println(char* string);

//Imprime un numero decimal positivo.
void printint(uint64_t value);

//Limpia la pantalla dejando el color de fondo default
void clearScreen();

//Getters del tamaño de la matriz (Ancho)
int getScreenWidth();

//Getters del tamaño de la matriz (Alto)
int getScreenHeight();

int strcmp(char * s1, char * s2);

int strlen(char * s);

//Funcion para cortar un string terminado en cero utilizando un delimitador provisto.
// Modifica el string.
int strtok(char * s, char delim, char * array[], int arraySize);

//Operacion potencia (Matematica)
int pow(int base, unsigned int exp);

//Funcion que convierte un string que tenga forma de numero hexadecimal,
// al numero que representa en decimal
uint64_t hexstringToInt(char * s);

//SysCall callers
//syscall 0:
extern int getTicksElapsed();

//syscall 1:
extern void drawPixel(unsigned int x, unsigned int y, int color);

//syscall 2:
extern void sysWrite(char * string, unsigned int font, unsigned int background);

//syscall 3:
extern char getChar();

//syscall 4:
extern void setCursorPos(unsigned int x, unsigned int y);

//syscall 5:
extern int verticalPixelCount();

//syscall 6:
extern int horizontalPixelCount();

//syscall 7:
extern uint8_t getTime(uint64_t timeDescriptor);

//syscall 8:
extern void sysBeep(uint32_t nFrequence, uint16_t time);

#endif