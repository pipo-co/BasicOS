#include <stdint.h>
#include <screenDriver.h>
#include <lib.h>
#include <interrupts.h>
#include <keyboardDriver.h>

#define ZERO_EXCEPTION_ID 0
#define INVALID_OPCODE_EXCEPTION_ID 6

static void zero_division(uint64_t * exceptionStackframe);
static void invalid_opcode(uint64_t * exceptionStackframe);

char * regs[] = {
	"R15: ", "R14: ", "R13: ", "R12: ", "R11: ", "R10: ", "R9: ",
	"R8: ", "RSI: ", "RDI: ", "RBP: ", "RDX: ", "RCX: ", "RBX: ",
	"RAX: ", "IP: ", "RSP: "
};

char buffer[5];

void printRegs(uint64_t * address){
	char buffer[50];
	for (int i = 0; i < 16; i++){
		printString(regs[i]);
		uintToBase(*(address + i), buffer, 16);
		println(buffer);
	}
	printString(regs[16]);
	uintToBase((uint64_t)(address + 15 + 3), buffer, 16); //RSP
	println(buffer);
}

void exceptionDispatcher(int exception, uint64_t * exceptionStackframe) {
	switch(exception){
	case ZERO_EXCEPTION_ID:
		zero_division(exceptionStackframe);
		break;
	case INVALID_OPCODE_EXCEPTION_ID:
		invalid_opcode(exceptionStackframe);
		break;
	}
}

static void zero_division(uint64_t * exceptionStackframe){
	println("Exception 0: Division by 0");
	printRegs(exceptionStackframe);
	exceptionStackframe[15] = 0x400000;
	exceptionStackframe[15+3] = 0x10CFC0;
	printString("Restarting shell in ");
	int aux;
	for (int i = 60; i > 0; i--){
		uintToBase(i, buffer, 10);
		printString(buffer);
		_hlt();
		aux = strilen(buffer);
		for (int i = 0; i < aux; i++)
			putchar('\b');
	}
	
}

static void invalid_opcode(uint64_t * exceptionStackframe){
	println("Exception 6: Invalid Opcode");
	printRegs(exceptionStackframe);
	while(1); //????
}