// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include <IORouter.h> 
#include <sem.h>
#include <pipe.h>

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
			//void routePrintStringf( char * string, unsigned int font, unsigned int background);
			routePrintStringf((char *) rsi, rdx, rcx);
			break;
        case 3:
			//char getchar();
            return getchar();
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
			//int kill(uint64_t pid);
			return kill(rsi);
		case 15:
			//uint64_t getPID();
			return getPID();
		case 16:
			//uint64_t initializeProccess(void (*function)(int , char **), uint8_t fg, int argc, char ** argv, uint16_t * stdFd);
			return initializeProccess((void (*)(int, char**))rsi, rdx, rcx, (char**)r8, (uint16_t *)r9);
		case 17:
			//void dumpScheduler();
			dumpScheduler();
			break;
		case 18:
			//void changeProccessPriority(uint64_t pid, uint8_t prority);
			changeProccessPriority(rsi, rdx);
			break;
		case 19:
			//int block(uint64_t pid);
			return block(rsi);
		case 20:
			//int unblock(uint64_t pid);
			return unblock(rsi);
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
		case 26:
			//int32_t openPipe(char * pipeName)
			return openPipe((char*)rsi);
		case 27:
			//int writePipe(uint16_t pipeId, char c)
			return writePipe(rsi, (char)rdx);
		case 28:
			//char readPipe(uint16_t pipe)
			return readPipe(rsi);
		case 29:
			//void closePipe(uint16_t pipe)
			closePipe(rsi);
			break;
		case 30:
			//void dumpPipes()
			dumpPipes();
			break;
		case 31:
			//int writeStringPipe(uint16_t pipeId, char * s)
			return writeStringPipe(rsi, (char*)rdx);
		case 32:
			//void wait(uint64_t pid);
			wait(rsi);
			break;
		case 33:
			//void skipTurn();
			skipTurn();
			break;
	}
	return 0;
}