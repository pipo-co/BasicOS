//keyboardDriver.h

#ifndef __KEYBOARDRIVER_H__
#define __KEYBOARDRIVER_H__

#define MAX_SC_TO_ASCII 58
#define LSHFT_DOWN 42
#define LSHFT_UP (LSHFT_DOWN + 128)
#define RSHFT_DOWN 54
#define RSHFT_UP (RSHFT_DOWN + 128)
#define CAPSLOCK_DOWN 58
#define ESC 27

//Permite verificar si existe un valor esperando en el registro del teclado
extern int keyboardActivated();

//Funcion asignada como rutina de atencion de la int 21h, producida por el IRQ1. 
//Guarda en el buffer del keyboardDriver el ascii asociado a la tecla presionada 
// (si es que tienen un ascii asociado).
int storeKey();

//Getter del flag de shift
int isShiftActivated();

//Getter del flag de capsLock
int isCapsActivated();

//Funcion asociada a la sysCall 3, devuelve el valor mas antiguo que haya en el buffer.
//El comportamiento del buffer es el de una cola (FIFO)
//Devuelve 0 si no habia tecla en el buffer
char getKey();

#endif