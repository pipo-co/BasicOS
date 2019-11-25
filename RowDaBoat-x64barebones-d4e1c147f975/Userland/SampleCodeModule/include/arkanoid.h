//arkanoid.h

#ifndef __ARKANOID_H__
#define __ARKANOID_H__

//Flag utilizado para decirle al juego si empezar una partida nueva o 
// seguir con la anterior, si es que existe.
enum gameMode{NEW_GAME=0, CONTINUE};

//Funcion utilizada para correr el juego. Utilizada por shell
void startArkanoid(enum gameMode mode);

//Getter para saber si hay una partida en curso.
int gameAlreadyStarted();

#endif