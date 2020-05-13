#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

uint64_t scheduler(uint64_t rsp);

//Starts with default (neutral) priority. Retorna el PID del proceso. Ante un error devuelve 0 (no existe PID 0).
//El nombre de la funcion se encuentra como primer elemento de argv, pero esto no le llega al nuevo proceso.
//stdFd debe ser un array de 2 elementos, el primero el fd de stdIn, y el segundo el de stdOut.
//Tambien se permite que stdFd sea NULL, se tomaran ambos valores como 0.
uint64_t initializeProccess(int (*function)(int , char **), uint8_t fg, int argc, char ** argv, uint16_t * stdFd);

int kill(uint64_t pid);

int block(uint64_t pid);

int unblock(uint64_t pid);

void skipTurn();

void exit();

uint64_t getPID();

void wait(uint64_t pid);

void dumpScheduler();

void dumpProcessFromPID(uint64_t pid);

void initScheduler();

void changeProccessPriority(uint64_t pid, uint8_t prority);

uint16_t getRunningProcessStdIn();

uint16_t getRunningProcessStdOut();

#endif
