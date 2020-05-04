#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

uint64_t scheduler(uint64_t rsp);

//Starts with default (neutral) priority. Retorna el PID del proceso. Ante un error devuelve 0 (no existe PID 0).
//El nombre de la funcion se encuentra como primer elemento de argv, pero esto no le llega al nuevo proceso.
//stdFd debe ser un array de 2 elementos, el primero el fd de stdIn, y el segundo el de stdOut.
//Tambien se permite que stdFd sea NULL, se tomaran ambos valores como 0.
uint16_t initializeProccess(int (*function)(int , char **), uint8_t fg, int argc, char ** argv, uint16_t * stdFd);

void kill(uint16_t pid);

void block(uint16_t pid);

void unblock(uint16_t pid);

void exit();

uint16_t getPID();

void dumpScheduler();

void dumpProcessFromPID(uint16_t pid);

void initScheduler();

void changeProccessPriority(uint16_t pid, uint8_t prority);

uint16_t getRunningProcessStdIn();

uint16_t getRunningProcessStdOut();

#endif
