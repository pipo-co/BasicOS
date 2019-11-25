//irqDispatcher.c
//Archivo provisto por la catedra
#include <timerTick.h>
#include <stdint.h>
#include <keyboardDriver.h>

static void int_20();
static void int_21();

void irqDispatcher(uint64_t irq) {
	switch (irq) {
		case 0:
			int_20();
			break;
		case 1:
			int_21();
			break;
	}
	return;
}

//Llamado a la rutina de atencion asocida
void int_20() {
	timer_handler();
}

//Llamado a la rutina de atencion asocida
void int_21() {
	storeKey();
}