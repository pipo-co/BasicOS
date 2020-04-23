#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

int32_t createSem(char * name, uint16_t initValue);

int semWait(uint16_t sem);

int semPost(uint16_t sem);

void removeSem(uint16_t sem);

void dumpSem();

#endif
