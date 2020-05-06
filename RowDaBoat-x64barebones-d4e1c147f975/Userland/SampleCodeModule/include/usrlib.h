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

int atoi(char * s);

extern void _hlt();

extern void _cli();

extern void _sti();

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

//syscall 9:
extern void * malloc2(uint32_t bytes);

//syscall 10:
extern int free2(void * ap);

//syscall 11:
extern uint32_t getDynamicMemLeft();

//syscall 12:
extern void dumpMM();

//syscall 13:
extern void exit();

//syscall 14:
extern void kill(uint16_t pid);

//syscall 15:
extern uint16_t getPID();

//syscall 16:
extern uint16_t initializeProccess(int (*function)(int , char **), uint8_t fg, int argc, char ** argv, uint16_t * stdFd);

//syscall 17:
extern void dumpScheduler();

//syscall 18
extern void changeProccessPriority(uint16_t pid, uint8_t prority);

//syscall 19
extern void block(uint16_t pid);

//syscall 20
extern void unblock(uint16_t pid);

//syscall 21
extern int32_t createSem(char * name, uint16_t initValue);

//syscall 22
extern int semWait(uint16_t sem);

//syscall 23
extern int semPost(uint16_t sem);

//syscall 24
extern void removeSem(uint16_t sem);

//syscall 25
extern void dumpSem();

//syscall 26
extern int32_t openPipe(char * pipeName);

//syscall 27
extern int writePipe(uint16_t pipe, char * s);

//syscall 28
extern char readPipe(uint16_t pipe);

//syscall 29
extern void closePipe(uint16_t pipe);

//syscall 30
extern void dumpPipes();

#endif