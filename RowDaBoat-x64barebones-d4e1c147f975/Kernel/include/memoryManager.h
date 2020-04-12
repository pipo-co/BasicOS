#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__

#include <stdint.h>
#include <stddef.h>

void initMM();

//Heavily inspired in C malloc
void * malloc2(uint32_t bytes);

int free2(void * ap);

uint32_t getAvailableMemory();

void printList();

#endif
