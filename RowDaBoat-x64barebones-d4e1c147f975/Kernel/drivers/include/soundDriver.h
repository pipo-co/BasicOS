//soundDriver.h
#ifndef __SOUND_DRIVER_H__
#define __SOUND_DRIVER_H__

//Implementacion del driver de video tomada de: 
//https://wiki.osdev.org/PC_Speaker, levemente modifica

//Funcion asociada a la sysCall 8, permite emitir un sondio con una frecuencia y duracion 
// seleccionada por el usuario.

void beep(uint32_t nFrequence, uint16_t time);

#endif