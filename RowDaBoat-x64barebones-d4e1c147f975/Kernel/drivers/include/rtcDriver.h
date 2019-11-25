//rtcDriver.h
#ifndef __RTC_DRIVER_H__
#define __RTC_DRIVER_H__

#define HOURS 04
#define MINUTES 02
#define SECONDS 00

#include <stdint.h>
//Funcion de assembler encargada de acceder a la posicion del rtc en el mata de 
// entrada/salida y obetener el tiempo asociado al time descriptor enviado (Hora, Minutos o Segundos).
extern uint8_t getRtc(uint64_t timeDescriptor);

//Funcion asociada a la sysCall 7, obtiene el valor pedido y lo convierte de BCD a decimal. 
uint8_t getTime(uint64_t timeDescriptor);

#endif