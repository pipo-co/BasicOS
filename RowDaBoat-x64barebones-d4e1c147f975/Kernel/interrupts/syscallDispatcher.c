//syscallDispatcher.c

#include <stdint.h>
#include <lib.h>
#include <timerTick.h>
#include <videoDriver.h>
#include <screenDriver.h>
#include <keyboardDriver.h>
#include <rtcDriver.h>
#include <soundDriver.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <sem.h>

//Funcion encargada de llamar a la funcion asociada a la systemCall llamada y pasarle 
// los parametros correctos.
uint64_t syscallDispatcher(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9){

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
		case 9:
			//void * malloc2(uint32_t bytes);
			return (uint64_t)malloc2(rsi);
		case 10:
			//int free2(void * ap);
			return free2((void*)rsi);
		case 11:
			//uint32_t getAvailableMemory();
			return getAvailableMemory();
		case 12:
			//void dumpMM();
			dumpMM();
			break;
		case 13:
			//void exit();
			exit();
			break;
		case 14:
			//void kill(uint16_t pid);
			kill(rsi);
			break;
		case 15:
			//uint16_t getPID();
			return getPID();
		case 16:
			//uint16_t initializeProccess(int (*function)(int , char **), char* name, uint8_t fg, int argc, char ** argv);
			return initializeProccess((int (*)(int, char**))rsi, (char*)rdx, rcx, r8, (char**)r9);
		case 17:
			//void dumpScheduler();
			dumpScheduler();
			break;
		case 18:
			//void changeProccessPriority(uint16_t pid, uint8_t prority);
			changeProccessPriority(rsi, rdx);
			break;
		case 19:
			//void block(uint16_t pid);
			block(rsi);
			break;
		case 20:
			//void unblock(uint16_t pid);
			unblock(rsi);
			break;
		case 21:
			//int32_t createSem(char * name, uint16_t initValue);
			return createSem((char*)rsi, rdx);
		case 22:
			//int semWait(uint16_t sem);
			return semWait(rsi);
		case 23:
			//int semPost(uint16_t sem);
			return semPost(rsi);
		case 24:
			//void removeSem(uint16_t sem);
			removeSem(rsi);
			break;
		case 25:
			//void dumpSem();
			dumpSem();
			break;
	}
	return 0;
}