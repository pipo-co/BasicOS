
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

extern int keyboardActivated();

int storeKey();

int isShiftActivated();

int isCapsActivated();

char getKey();

int getScancode();

#endif