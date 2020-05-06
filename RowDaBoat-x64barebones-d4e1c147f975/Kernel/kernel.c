//kernel.c

#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <idtLoader.h>
#include <exceptions.h>
#include <screenDriver.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <sem.h>
#include <pipe.h>
#include <keyboardDriver.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;
extern void _hlt();

static const uint64_t PageSize = 0x1000;

static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;
static void * const heapBaseAddress = (void*)0x600000;	//Posicion de memoria 6MiB, 1 MiB despues despues del comienzo de la seccion de datos de Userland.
#define HEAP_SIZE (128 * 1024 * 1024) //128 MiB 

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize){
	memset(bssAddress, 0, bssSize);
}

void * getStackBase(){
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary(){
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);

	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}

int main(){
	//Funciones de inicializacion de video, de la IDT, del Memory Manager, del scheduler y del controlador de excepciones.
	init_screen();

	load_idt();

	initMM(heapBaseAddress, HEAP_SIZE);

	if(initPipes() == -1)
		return -1;

	if(initKeyboardDriver() == -1)
		return -1;

	initExceptionHandler((uint64_t)sampleCodeModuleAddress, getSP()); 

	initScheduler();

	char * argv[] = {"Sample Code Module"};
	initializeProccess(sampleCodeModuleAddress, 1, 1, argv, NULL);

	_hlt(); //Hace el sti y hlt. Primer TimerTick

	println("No deberiamos llegar aca");
	return 0;
}