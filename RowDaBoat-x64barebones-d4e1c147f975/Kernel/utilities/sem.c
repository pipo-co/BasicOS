#include <stdint.h>
#include <sem.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define MAX_SEMAPHORE 100

typedef struct u16Node{
    uint16_t num;
    struct u16Node * next;
}u16Node;

typedef struct u16Queue{
    u16Node * first;
    u16Node * last; 
}u16Queue;

typedef struct semaphore{
    uint16_t counter;
    char * name;
    u16Queue blockedProcessesPid;
    uint16_t dependantProcessesCount;
    uint8_t active;
}semaphore_t;

typedef struct semaphores{
    semaphore_t semArray[MAX_SEMAPHORE];
    uint16_t firstInactive;
    uint16_t size;
}semaphores_t;

static int32_t getSemIndexFromName(char * name);
static void initializeSem(char * name, uint16_t initValue);
static int isValidSem(uint16_t sem);
static void dumpSemaphore(semaphore_t sem);

//u16Queue Methods
static int u16Push(u16Queue * q, uint16_t n);
static uint16_t u16Pop(u16Queue * q);
static int u16QueueIsEmpty(u16Queue * q);


static semaphores_t semaphores;


int32_t createSem(char * name, uint16_t initValue){
    if(name == NULL)
        return -1;

    int index = getSemIndexFromName(name);

    if(index != -1){
        semaphores.semArray[index].dependantProcessesCount++;
        return index;
    }

    if(semaphores.size >= MAX_SEMAPHORE)
        return -1;

    initializeSem(name, initValue);

    return index;      
}

int semWait(uint16_t sem){
    if(!isValidSem(sem))
        return -1;

    if(semaphores.semArray[sem].counter > 0){
        semaphores.semArray[sem].counter--;
        return 0;
    }
    
    uint16_t runningProcessPID = getPID();

    if(u16Push(&semaphores.semArray[sem].blockedProcessesPid, runningProcessPID) == -1)
        return -1;

    block(runningProcessPID);

    return 0;
}

int semPost(uint16_t sem){
    if(!isValidSem(sem))
        return -1;

    if(!u16QueueIsEmpty(&semaphores.semArray[sem].blockedProcessesPid))
        unblock(u16Pop(&semaphores.semArray[sem].blockedProcessesPid));
    else
        semaphores.semArray[sem].counter++;
    
    return 0;
}

void removeSem(uint16_t sem){
    if(!isValidSem(sem))
        return;
    
    semaphores.semArray[sem].dependantProcessesCount--;

    if(semaphores.semArray[sem].dependantProcessesCount > 0)
        return;

    if(!u16QueueIsEmpty(&semaphores.semArray[sem].blockedProcessesPid))
        println("Error Semaphores - Closed Sem With Blocked Processes");

    semaphores.semArray[sem].active = 0;

    semaphores.size--;

    if(sem < semaphores.firstInactive)
        semaphores.firstInactive = sem;
}

void dumpSemaphores(){
    uint16_t activeCount = 0;
    printString("Total semaphores: "); printint(semaphores.size); putchar('\n');

    for(uint16_t i = 0; activeCount < semaphores.size; i++){
        if(semaphores.semArray[i].active){
            activeCount++;
            printString("Semaphore Number: "); printint(activeCount); putchar('\n');
            printString("Code: "); printint(i); putchar(' ');
            dumpSemaphore(semaphores.semArray[i]);
        }
    }
}

static int32_t getSemIndexFromName(char * name){
    uint16_t i = 0;

    for(uint16_t activeSemCount = 0; activeSemCount < semaphores.size; i++){
        if(semaphores.semArray[i].active){
            activeSemCount++;
            if(strcmp(name,semaphores.semArray[i].name))
                return i;
        }
    }
    return -1;
}

static void initializeSem(char * name, uint16_t initValue){
    uint16_t index = semaphores.firstInactive;

    semaphores.semArray[index].active = 1;
    semaphores.semArray[index].name = name;
    semaphores.semArray[index].counter = initValue;
    semaphores.semArray[index].dependantProcessesCount = 1;
    semaphores.semArray[index].blockedProcessesPid.first = NULL; //Deberian ser innecesarios, pero por las dudas...
    semaphores.semArray[index].blockedProcessesPid.last = NULL;
    
    semaphores.size++;

    for(uint16_t i = index + 1; i < MAX_SEMAPHORE; i++){
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

static void dumpSemaphore(semaphore_t sem){
    printString("Name: "); printString(sem.name);

    printString(" Counter: "); printint(sem.counter);

    printString(" Processes Dependant on Semaphore: "); printint(sem.dependantProcessesCount);

    (sem.active)? printString(" Is Active") : printString(" Is Not Active(PROBLEM)"); putchar('\n');

    if(!u16QueueIsEmpty(&sem.blockedProcessesPid)){
        println("Processes Blocked by Semaphore:");

        for(u16Node* iter = sem.blockedProcessesPid.first; iter != NULL; iter = iter->next){
            putchar('\t'); dumpProcessFromPID(iter->num);
        }
    }
}

//u16Queue Logic
static int u16Push(u16Queue * q, uint16_t n){
    if(q == NULL)
        return -1;

    u16Node * node = malloc2(sizeof(u16Node));
    if(node == NULL)
        return -1;

    node->num = n; 

    if(q->first == NULL)
        q->first = node;
    else
        q->last->next = node;

    q->last = node;
    node->next = NULL;

    return 0;
}

static uint16_t u16Pop(u16Queue * q){
    if(q == NULL || u16QueueIsEmpty(q))
        return 0; //Manejo de error incorrecto

    u16Node * ans = q->first;

    if(q->last == ans)
        q->last = NULL;

    q->first = q->first->next;

    uint16_t pid = ans->num;

    free2(ans);

    return pid;
}

static int u16QueueIsEmpty(u16Queue * q){
    return q == NULL || q->first == NULL;
}