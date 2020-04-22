#include <stdint.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define PRIORITY_COUNT 5
#define TIME_MULT 2
#define PROCCESS_STACK_SIZE (8 * 1024 - sizeof(proccessNode)) //8 KiB
#define DEFAULT_PRIORITY (PRIORITY_COUNT/2)

//Sempahore
#define MAX_SEMAPHORE 100


enum states{READY, BLOCKED, KILLED};
typedef struct proccess_t{
    char * name;
    uint16_t pid;
    uint64_t rsp;
    uint64_t rbp;
    uint8_t fg;
    uint8_t priority;
    enum states state;
}proccess_t;

typedef struct proccessNode{
    proccess_t proccess;
    struct proccessNode * next;
}proccessNode;

typedef struct{
    proccessNode * first;
    proccessNode * last; 
}proccessNodeQueue;

static proccessNodeQueue activeProccesses;
static uint16_t readyCount;

static proccessNode * runningProccessNode;
static uint64_t runtimeLeft;
static uint16_t pidCounter = 1;

static proccessNode * dummyProcessNode;

static void removeProccess(proccessNode * node);
static int isEmpty(proccessNodeQueue * q);
static void push(proccessNodeQueue * q, proccessNode * node);
static void pushActP(proccessNodeQueue * q, proccessNode * node);
static proccessNode* pop(proccessNodeQueue * q);
static proccessNode* popActP(proccessNodeQueue * q);
static proccessNode * getProccessNodeFromPID(uint16_t pid);
static uint64_t swapProccess(uint64_t rsp);
static void changeProccessState(uint16_t pid, enum states state);
static void createProccess(proccessNode * node, char* name, uint8_t fg, uint8_t prority);
static uint16_t getNewPID();
static int dummyFunction(int argc, char ** argv);

static void dumpProccess(proccess_t p);

extern void _hlt();

typedef struct stackFrame{ 
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rsi;   //argv
    uint64_t rdi;   //argc
    uint64_t rbp;
    uint64_t rdx;   //function
    uint64_t rcx;   //pid
    uint64_t rbx;
    uint64_t rax;
    uint64_t rip;   //loader
    uint64_t cs;    //0x8
    uint64_t rflags;//0x202    
    uint64_t rsp;   //rbp
    uint64_t ss;    //0
} stackFrame;


//Semaphore
typedef struct semaphore{
    uint16_t counter;
    char * name;
    proccessNodeQueue blockedProcess;
    uint16_t dependantProcessesCount;
    uint8_t active;
}semaphore_t;

typedef struct semaphores{
    semaphore_t semArray[MAX_SEMAPHORE];
    uint16_t firstInactive;
    uint16_t size;
}semaphores_t;

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

void semWait(uint16_t sem){
    if(!isValidSem(sem))
        return;

    if(semaphores.semArray[sem].counter > 0){
        semaphores.semArray[sem].counter--;
        return;
    }
    
    push(&semaphores.semArray[sem].blockedProcess, runningProccessNode);
    block(runningProccessNode->proccess.pid);
}

int32_t semPost(uint16_t sem){
    if(!isValidSem(sem))
        return;

    if(!isEmpty(&semaphores.semArray[sem].blockedProcess))
        unblock(pop(&semaphores.semArray[sem].blockedProcess)->proccess.pid);
    else
        semaphores.semArray[sem].counter++;
}

void removeSem(uint16_t sem){
    if(!isValidSem(sem))
        return;
    
    semaphores.semArray[sem].dependantProcessesCount--;

    if(semaphores.semArray[sem].dependantProcessesCount > 0)
        return;

    semaphores.semArray[sem].active = 0;

    if(sem < semaphores.firstInactive)
        semaphores.firstInactive = sem;
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
    semaphores.semArray[index].blockedProcess.first = NULL;
    semaphores.semArray[index].blockedProcess.last = NULL;
    
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

//Scheduler

void initScheduler(){
    //Initialize Dummy Process
    initializeProccess(dummyFunction, "Dummy Process", 0, 0, NULL); //Mete al dummy process en la cola
    dummyProcessNode = popActP(&activeProccesses); //Lo saco de la cola (es el unico Pc en ella)
    dummyProcessNode->proccess.priority = PRIORITY_COUNT; //Para que su runtime sea 0
}

uint64_t scheduler(uint64_t rsp){
    if(runningProccessNode->proccess.state == READY && runtimeLeft > 0){
        runtimeLeft--;
        return rsp;
    }
    return swapProccess(rsp);
}

static uint64_t swapProccess(uint64_t rsp){

    if(runningProccessNode != NULL){ //La primera vez ignoramos el update del stack
            runningProccessNode->proccess.rsp = rsp;
            
        if(runningProccessNode->proccess.pid != dummyProcessNode->proccess.pid){ //Si estaba corriendo dummy, no hay que pushear
            //BLOCKED CASE MISSING ?
            if(runningProccessNode->proccess.state == KILLED)
                removeProccess(runningProccessNode);
            else
                pushActP(&activeProccesses, runningProccessNode);
        }
    }

    if(readyCount > 0){

        runningProccessNode = popActP(&activeProccesses);
        while(runningProccessNode->proccess.state != READY){

            if(runningProccessNode->proccess.state == KILLED)
                removeProccess(runningProccessNode);
            else if(runningProccessNode->proccess.state == BLOCKED)
                pushActP(&activeProccesses, runningProccessNode);
            
            runningProccessNode = popActP(&activeProccesses);
        }
    }else
        runningProccessNode = dummyProcessNode;

    runtimeLeft = (PRIORITY_COUNT - runningProccessNode->proccess.priority) * TIME_MULT; //Heuristica

    return runningProccessNode->proccess.rsp;
}

uint16_t initializeProccess(int (*function)(int , char **), char* name, uint8_t fg, int argc, char ** argv){
    proccessNode * node = malloc2(PROCCESS_STACK_SIZE + sizeof(proccessNode));
    //dumpMM();
    if(node == NULL)
        return 0;

    createProccess(node, name, fg, DEFAULT_PRIORITY);
    pushActP(&activeProccesses, node);
    // if(node->proccess.pid != 1)
    //     dumpScheduler();

    stackFrame newSF;
    newSF.ss = 0;
    newSF.rsp = node->proccess.rbp;
    newSF.rflags = 0x202;
    newSF.cs = 0x8;
    newSF.rip = (uint64_t) loader2;
    newSF.rdi = argc;
    newSF.rsi = (uint64_t) argv;
    newSF.rdx = (uint64_t) function;
    newSF.rcx = node->proccess.pid;

    memcpy((void *)(node->proccess.rsp), &newSF, sizeof(stackFrame));
    return node->proccess.pid;
}

static void createProccess(proccessNode * node, char* name, uint8_t fg, uint8_t prority){
    proccess_t * p = &node->proccess;

    p->rbp = (uint64_t)node + PROCCESS_STACK_SIZE + sizeof(proccessNode) - sizeof(uint64_t);
    p->rsp = (uint64_t)(node->proccess.rbp - sizeof(stackFrame));
    p->name = name;
    p->pid = getNewPID();
    p->fg = fg;
    p->priority = prority;
    p->state = READY;
}

static uint16_t getNewPID(){
    pidCounter++;
    return pidCounter - 1;
}

static void removeProccess(proccessNode * node){ //Faltan cosas capaz
    free2(node);
}

static void push(proccessNodeQueue * q, proccessNode * node){
    if(q == NULL || node == NULL)
        return;

    if(q->first == NULL)
        q->first = node;
    else
        q->last->next = node;

    q->last = node;
    node->next = NULL;
}

static void pushActP(proccessNodeQueue * q, proccessNode * node){
    push(q,node);
    
    if(node->proccess.state == READY)
        readyCount++;
}

static proccessNode* pop(proccessNodeQueue * q){
    if(q == NULL || isEmpty(q))
        return NULL;

    proccessNode* ans = q->first;

    if(q->last == ans)
        q->last = NULL;

    q->first = q->first->next;

    return ans;
}

static proccessNode* popActP(proccessNodeQueue * q){
    proccessNode* ans = pop(q);
    if(ans->proccess.state == READY)
        readyCount--;
    return ans;
}

static int isEmpty(proccessNodeQueue * q){
    return q == NULL || q->first == NULL;
}

static proccessNode * getProccessNodeFromPID(uint16_t pid){
        for(proccessNode* iter = activeProccesses.first; iter != NULL; iter = iter->next){
            if(iter->proccess.pid == pid)
                return iter;
        }
    return NULL;
}

void loader2(int argc, char *argv[], int (*function)(int , char **)){
    function(argc, argv);
    exit();
}

static void changeProccessState(uint16_t pid, enum states state){
    if(pid == dummyProcessNode->proccess.pid)
        return;

    if(runningProccessNode->proccess.pid == pid){
        runningProccessNode->proccess.state = state;
        return;
    }

    proccessNode * node = getProccessNodeFromPID(pid);
    if(node != NULL && node->proccess.state != KILLED){

        if(node->proccess.state != READY && state == READY)
            readyCount++;
        else if(node->proccess.state == READY && state != READY)
            readyCount--;

        node->proccess.state = state;
    }
}

void changeProccessPriority(uint16_t pid, uint8_t priority){
    if(priority >= PRIORITY_COUNT)
        priority = PRIORITY_COUNT - 1;

    proccessNode* node;
    if(runningProccessNode->proccess.pid == pid)
        node = runningProccessNode;
    else 
        node = getProccessNodeFromPID(pid);

    if(node == NULL)
        return;

    node->proccess.priority = priority;
}

void exit(){
    kill(runningProccessNode->proccess.pid);
    _hlt();
}

void kill(uint16_t pid){ 
    changeProccessState(pid, KILLED);

    if(pid == runningProccessNode->proccess.pid) // Es lo mismo que exit
        _hlt();
}

void block(uint16_t pid){
    changeProccessState(pid, BLOCKED);

    if(pid == runningProccessNode->proccess.pid) // Espero a que el scheduler me saque
        _hlt();
}

void unblock(uint16_t pid){
    changeProccessState(pid, READY);
}

uint16_t getPID(){
    return runningProccessNode->proccess.pid;
}

void dumpScheduler(){
    uint16_t totalP = 1; //El running proccess

    if(runningProccessNode != NULL){
        println("Current Running Proccess:");
        dumpProccess(runningProccessNode->proccess);
        printString("Runetime Left (in Ticks): ");
        printint(runtimeLeft);
        putchar('\n');
    }

    println("Active Processes:");
    for(proccessNode* iter = activeProccesses.first; iter != NULL; iter = iter->next){
        dumpProccess(iter->proccess);
        totalP++;
    }

    printString("Total Proccesses in System: ");
    printint(totalP);
    putchar('\n');

    printString("Total Proccesses Created: ");
    printint(pidCounter - 1);
    putchar('\n');
}

static void dumpProccess(proccess_t p){
    printString("Name: ");
    printString(p.name);
    printString(" PID: ");
    printint(p.pid);
    printString(" RSP: 0x");
    printhex(p.rsp);
    printString(" RBP: 0x");
    printhex(p.rbp);
    (p.fg)? printString(" Foreground ") : printString(" Background ");
    printString("Priority: ");
    printint(p.priority);
    printString(" State: ");
    switch(p.state){
        case READY:
            printString("READY"); break;
        case KILLED:
            printString("KILLED"); break;
        case BLOCKED:
            printString("BLOCKED"); break;
    }
    putchar('\n');
}

static int dummyFunction(int argc, char ** argv){
    while(1){
        println("Dummy");
        _hlt();
    }
    return 0;
}

