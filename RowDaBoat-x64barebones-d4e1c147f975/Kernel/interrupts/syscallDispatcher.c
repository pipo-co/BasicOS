//syscallDispatcher.c

#include <stdint.h>
#include <timerTick.h>
#include <videoDriver.h>
#include <screenDriver.h>
#include <keyboardDriver.h>
#include <rtcDriver.h>
#include <soundDriver.h>

//Funcion encargada de llamar a la funcion asociada a la systemCall llamada y pasarle 
// los parametros correctos.
uint64_t syscallDispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx){
	switch(rdi){
        case 0:
			//int ticks_elapsed();
            return ticks_elapsed(); 
		case 1:
			//void drawPixel(unsigned int x, unsigned int y, int color);
			drawPixel(rsi, rdx, rcx); 
			break;
		case 2:
			//void printStringf( char * string, unsigned int font, unsigned int background);
			printStringf((char *)rsi, rdx, rcx);
			break;
        case 3:
			//char getKey();
            return getKey();
		case 4:
			//void setCursorPos(unsigned int x, unsigned int y);
			setCursorPos(rsi, rdx);
			break;
		case 5:
			//int verPixelCount();
			return verPixelCount();
		case 6:
			//int horPixelCount()
			return horPixelCount();
		case 7:
			//uint8_t getTime(uint64_t timeDescriptor)
			return getTime(rsi);
		case 8:
			//void beep(uint32_t nFrequence, uint16_t time);
			beep(rsi,rdx);
			break;
	}
	return 0;
}