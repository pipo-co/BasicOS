#include <stdint.h>
#include <timerTick.h>
#include <videoDriver.h>
#include <screenDriver.h>
#include <keyboardDriver.h>


uint64_t syscallDispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx){
	switch(rdi){
        case 0:
            return ticks_elapsed();
		case 1:
			drawPixel(rsi, rdx, rcx);
			break;
		case 2:
			printStringf((char *)rsi, rdx, rcx);
			break;
        case 3:
            return getKey();
		case 4:
			setCursorPos(rsi, rdx);
			break;
		case 5:
			return verPixelCount();
		case 6:
			return horPixelCount();
	}
	return 0;
}