//kernel.c
//Archivo original, el mayor cambio fue sacar todos los print que 
// realizaba al inicializar el kernel.

#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <idtLoader.h>
#include <exceptions.h>
#include <screenDriver.h>
#include <memoryManager.h>
#include <scheduler.h>

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

int prueba(int a, char ** b){
	println("Llegamos!!!");
	while(1);
}

int main(){
	//Funciones de inicializacion de video, de la IDT, del Memory Manager y del controlador de excepciones.
	init_screen();
	load_idt();
	initMM(heapBaseAddress, HEAP_SIZE);	
	initExceptionHandler((uint64_t)sampleCodeModuleAddress, getSP()); 
	//return ((EntryPoint)sampleCodeModuleAddress)();
	println("hola");
	initScheduler();
	initializeProccess(prueba, "Prueba", 1, 0, NULL);
	_hlt(); //Hace el sti y hlt
	println("No deberiamos llegar aca");
	return 0;
}
