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

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;

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
	//Funciones de inicializacion de video, de la IDT, del Memory Manager y del controlador de excepciones.
	init_screen();
	load_idt();
	initMM();
	

	//Prebas Malloc
	printList();
    int* var1 = malloc2(1000);
    int* var2 = malloc2(1000);
    int* var3 = malloc2(1000);
    int* var4 = malloc2(1000);
    int* var5 = malloc2(1000);
    int* var6 = malloc2(1000);
    int* var7 = malloc2(1000);
    int* var8 = malloc2(1000);
    int* var9 = malloc2(1000);
    int* var10 = malloc2(1000);
	int* var11 = malloc2(8387967 * 16);
	if(var11 == NULL)
		println("Dio NULL");

	printList();


    free2(var9);
    
    printList();
    free2(var7);

	printList();
	free2(var9);

    printList();
    free2(var5);
    printList();

    
    //free2((node*)var9 + 1);

    free2(var3);
	printList();
    free2(var6);
    free2(var10);
    printList();
    free2(var1);

    free2(var4);

    free2(var2);
	printList();

    free2(var8);
	printList();

    //free2(heapBase + (first->s.size - 4) * BLOCK_SIZE + 16);
	
	//initExceptionHandler((uint64_t)sampleCodeModuleAddress, getSP()); 
	return 0;//((EntryPoint)sampleCodeModuleAddress)();
}
