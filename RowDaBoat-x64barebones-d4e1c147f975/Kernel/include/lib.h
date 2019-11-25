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

//Funcion auxiliar que permite obtener el valor del SP.
extern uint64_t getSP();

int strilen(char * s);

#endif