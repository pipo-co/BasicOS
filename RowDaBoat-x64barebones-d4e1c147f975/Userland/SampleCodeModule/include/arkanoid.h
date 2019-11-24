#ifndef __ARKANOID_H__
#define __ARKANOID_H__

enum gameMode{NEW_GAME=0, CONTINUE};

void startArkanoid(enum gameMode mode);

int gameAlreadyStarted();

#endif