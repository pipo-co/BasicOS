//lib.h
//libreria general de kernel

#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void * memset( void * destination, int32_t character, uint64_t length);
void * memcpy( void * destination, const void * source, uint64_t length);

extern char *cpuVendor( char *result);

//Funcion proveniente de naiveConsole.
uint32_t uintToBase( uint64_t value, char * buffer, uint32_t base);

//Funcion para la realizacion de un in desde el mapa de entarda/salida.
extern uint8_t inb( uint16_t port);

//Funcion para la realizacion de un out al mapa de entarda/salida.
extern void outb( uint16_t port, uint8_t value);

extern void _hlt();

extern void _cli();

extern void _sti();

//Funcion auxiliar que permite obtener el valor del SP.
extern uint64_t getSP();

//Funciones para evitar que dos procesos ejecuten las misma region simultaneamente
extern void enter_critical_region(uint8_t * lock);

extern void leave_critical_region(uint8_t * lock);

int strilen(char * s);

void printint(uint64_t value);

void printhex(uint64_t value);

//Calcular el logaritmo en base 2 truncado a entero.
uint8_t intLog2(uint64_t n);

//Boolean. True if both strings are equal.
int strcmp(char * s1, char * s2);

int strcat(char *dest , char *source);

void strncpy(char * dest, char * source, uint32_t n);
  

#endif