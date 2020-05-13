// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <sem.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define MAX_SEMAPHORE 100
#define NAME_SIZE 20
#define MAX_BLOCKED_PROCESSES 20

typedef struct pidQueue{
    uint64_t queue[MAX_BLOCKED_PROCESSES];
    uint16_t head;
    uint16_t tail;
    uint16_t size; 
}pidQueue_t;

typedef struct semaphore{
    uint16_t counter;
    char name[NAME_SIZE];
    pidQueue_t blockedProcessesPidQueue;
    uint16_t dependantProcessesCount;
    uint8_t active;
    uint8_t inUse;
}semaphore_t;

typedef struct semaphores{
    semaphore_t semArray[MAX_SEMAPHORE];
    uint16_t firstInactive;
    uint16_t size;
}semaphores_t;

static int isValidSem(uint16_t sem);
static void dumpSemaphore(semaphore_t * sem);
static int32_t getSemIndexFromName(char * name);
static uint16_t initializeSem(char * name, uint16_t initValue);
static void updateFirstInactive(uint16_t oldValue);

//Static Circular Queue Methods
static int pidEnqueue(pidQueue_t * q, uint64_t pid);
static uint64_t pidDequeue(pidQueue_t * q);
static int isQueueEmpty(pidQueue_t * q);
static void initializePidQueue(pidQueue_t * q);
static void dumpBlockedProcesses(pidQueue_t * q);

static semaphores_t semaphores;

static uint8_t semCreationLock; //Only one Sem can be created or opened at a time.

int32_t createSem(char * name, uint16_t initValue){
    if(name == NULL)
        return -1;

    enter_critical_region(&semCreationLock); //Unica critical region larga

    int index = getSemIndexFromName(name);

    if(index != -1){
        semaphores.semArray[index].dependantProcessesCount++;
        leave_critical_region(&semCreationLock);
        return index;
    }

    if(semaphores.size >= MAX_SEMAPHORE){
        leave_critical_region(&semCreationLock);
        return -1;
    }

    index = initializeSem(name, initValue);    

    leave_critical_region(&semCreationLock);

    return index;      
}

int semWait(uint16_t sem){
    if(!isValidSem(sem))
        return -1;

    enter_critical_region(&semaphores.semArray[sem].inUse);

    if(semaphores.semArray[sem].counter > 0){
        semaphores.semArray[sem].counter--;
        leave_critical_region(&semaphores.semArray[sem].inUse);
        return 0;
    }
    
    uint64_t runningProcessPID = getPID();

    if(pidEnqueue(&semaphores.semArray[sem].blockedProcessesPidQueue, runningProcessPID) == -1){
        leave_critical_region(&semaphores.semArray[sem].inUse);
        return -1;
    }

    leave_critical_region(&semaphores.semArray[sem].inUse);

    block(runningProcessPID);

    return 0;
}

int semPost(uint16_t sem){
    if(!isValidSem(sem))
        return -1;

    enter_critical_region(&semaphores.semArray[sem].inUse);

    if(!isQueueEmpty(&semaphores.semArray[sem].blockedProcessesPidQueue))
        unblock(pidDequeue(&semaphores.semArray[sem].blockedProcessesPidQueue));
    else
        semaphores.semArray[sem].counter++;
    
    leave_critical_region(&semaphores.semArray[sem].inUse);

    return 0;
}

void removeSem(uint16_t sem){
    if(!isValidSem(sem))
        return;

    enter_critical_region(&semaphores.semArray[sem].inUse);

    if(!semaphores.semArray[sem].active){
        leave_critical_region(&semaphores.semArray[sem].inUse);
        return;
    }
    
    semaphores.semArray[sem].dependantProcessesCount--;

    if(semaphores.semArray[sem].dependantProcessesCount > 0){
        leave_critical_region(&semaphores.semArray[sem].inUse);
        return;
    }

    if(!isQueueEmpty(&semaphores.semArray[sem].blockedProcessesPidQueue))
        println("Error Semaphores - Closed Sem With Blocked Processes");

    semaphores.semArray[sem].active = 0;

    semaphores.size--;

    if(sem < semaphores.firstInactive)
        semaphores.firstInactive = sem;

    leave_critical_region(&semaphores.semArray[sem].inUse);
}

void dumpSem(){
    uint16_t activeCount = 0;
    printString("Total semaphores: "); printint(semaphores.size); putchar('\n');

    for(uint16_t i = 0; activeCount < semaphores.size; i++){
        if(semaphores.semArray[i].active){
            activeCount++;
            printString("Code: "); printint(i); putchar(' ');
            dumpSemaphore(&semaphores.semArray[i]);
        }
    }
}

void dumpProcessesBlockedBySem(uint16_t sem){
    if(!isValidSem(sem))
        return;

    dumpBlockedProcesses(&semaphores.semArray[sem].blockedProcessesPidQueue);
}

static int32_t getSemIndexFromName(char * name){
    uint16_t i = 0;

    for(uint16_t activeSemCount = 0; activeSemCount < semaphores.size; i++){
        if(semaphores.semArray[i].active){
            activeSemCount++;
            if(strcmp(name, semaphores.semArray[i].name))
                return i;
        }
    }
    return -1;
}

static uint16_t initializeSem(char * name, uint16_t initValue){
    uint16_t index = semaphores.firstInactive;

    semaphores.semArray[index].active = 1;
    strncpy(semaphores.semArray[index].name, name, NAME_SIZE - 1);
    semaphores.semArray[index].counter = initValue;
    semaphores.semArray[index].dependantProcessesCount = 1;
    semaphores.semArray[index].inUse = 0;
    initializePidQueue(&semaphores.semArray[index].blockedProcessesPidQueue);
    
    semaphores.size++;

    updateFirstInactive(index);

    return index;
}

static void updateFirstInactive(uint16_t oldValue){
    for(uint16_t i = oldValue + 1; i < MAX_SEMAPHORE; i++){
        if(!semaphores.semArray[i].active){
            semaphores.firstInactive = i;
            return;
        }
    }
    semaphores.firstInactive = MAX_SEMAPHORE; //The array is full
}

static int isValidSem(uint16_t sem){
    return sem < MAX_SEMAPHORE && semaphores.semArray[sem].active;
}

static void dumpSemaphore(semaphore_t * sem){
    printString("Name: "); printString(sem->name);

    printString(" Counter: "); printint(sem->counter);

    printString(" Processes Dependant on Semaphore: "); printint(sem->dependantProcessesCount); putchar('\n');

    if(!isQueueEmpty(&sem->blockedProcessesPidQueue)){
        println("Processes Blocked by Semaphore:");

        dumpBlockedProcesses(&sem->blockedProcessesPidQueue);
    }
}

static void dumpBlockedProcesses(pidQueue_t * q){
    for(uint16_t i = q->head; i != q->tail; i = i + 1 % MAX_BLOCKED_PROCESSES){
        putchar('\t'); dumpProcessFromPID(q->queue[i]);
    }
}

static int pidEnqueue(pidQueue_t * q, uint64_t pid){
    if(q->size >= MAX_BLOCKED_PROCESSES)
        return -1;

    q->queue[q->tail] = pid;
        
    q->tail = (q->tail + 1) % MAX_BLOCKED_PROCESSES;

    q->size++;

    return 0;
}

static uint64_t pidDequeue(pidQueue_t * q){
    if(isQueueEmpty(q))
        return 0;

    uint16_t pid = q->queue[q->head];
    
    q->head = (q->head + 1) % MAX_BLOCKED_PROCESSES;

    q->size--;

    return pid;
}

static int isQueueEmpty(pidQueue_t * q){
    return q->size == 0;
}

static void initializePidQueue(pidQueue_t * q){
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}