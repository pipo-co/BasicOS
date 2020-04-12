#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__

// BUDDY o FREELIST

#include <stdint.h>
#include <stddef.h>

void initMM(void* init, unsigned blockCount);

//Heavily inspired in C malloc
void * malloc2(unsigned bytes);

int free2(void * ap);

size_t getAvailableMemory();

#endif
