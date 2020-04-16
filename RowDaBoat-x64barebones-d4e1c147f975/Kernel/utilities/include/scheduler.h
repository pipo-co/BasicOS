#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

uint64_t * scheduler(uint64_t * rsp);

int initializeProccess(int (*function)(int , char **), char* name, uint8_t fg, uint8_t priority, int argc, char ** argv);

void loader(int argc, char const *argv[], int (*function)(int , char **), int pid);

void kill(uint8_t pid);

#endif
