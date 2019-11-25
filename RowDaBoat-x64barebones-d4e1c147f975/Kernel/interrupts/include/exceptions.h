//exceptions.h
#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <stdint.h>

//Funcion para asignar los valores utilizados a la hora de continuar la ejecucion
// luego de una excepcion
void initExceptionHandler(uint64_t ipAdress, uint64_t stackAdress);

#endif