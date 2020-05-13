// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define MAX_PRIORITY 4
#define TIME_MULT 2
#define PROCCESS_STACK_SIZE (8 * 1024 - sizeof(proccessNode)) //8 KiB
#define DEFAULT_BACKGROUND_PRIORITY 0
#define DEFAULT_FOREGROUND_PRIORITY MAX_PRIORITY

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

enum states{READY, BLOCKED, KILLED};
typedef struct proccess_t{
    char * name;
    uint64_t pid;       //No existe pid 0, reservado para error.
    uint64_t rsp;
    uint64_t rbp;
    uint8_t fg;
    uint8_t priority;
    uint16_t stdIn;     // 0 significa teclado/pantalla.
    uint16_t stdOut;
    uint64_t pPid;
    uint8_t awakeParent;
    enum states state;
}proccess_t;

typedef struct proccessNode{
    proccess_t proccess;
    struct proccessNode * next;
}proccessNode;

typedef struct{
    proccessNode * first;
    proccessNode * last;
    uint16_t readyCount; 
}proccessNodeQueue;

static uint64_t swapProccess(uint64_t rsp);

static void removeProccess(proccessNode * node);
static proccessNode * getProccessNodeFromPID(uint64_t pid);
static int changeProccessState(uint64_t pid, enum states state);
static void createProccess(proccessNode * node, char* name, uint8_t fg, uint16_t * stdFd);
static char ** copyArguments(char ** newArgv, int argc, char ** argv);
static uint64_t getNewPID();
static void dumpProccess(proccess_t p);

static void loader2(int argc, char *argv[], void (*function)(int , char **));
static void dummyFunction(int argc, char ** argv);

//Process Queue Methods
static int isProcessQueueEmpty(proccessNodeQueue * q);
static void processEnqueue(proccessNodeQueue * q, proccessNode * node);
static proccessNode* processDequeue(proccessNodeQueue * q);
static uint64_t dumpQueueProcesses(proccessNodeQueue * q);


static proccessNodeQueue activeProccesses;

static proccessNode * runningProccessNode;
static uint64_t runtimeLeft;
static uint64_t pidCounter = 1;

static proccessNode * dummyProcessNode;

void initScheduler(){
    // Initialize Dummy Process
    char * argv[] = {"Dummy Process"};
    initializeProccess(dummyFunction, 0, 1, argv, NULL); // Mete al dummy process en la cola
    dummyProcessNode = processDequeue(&activeProccesses); // Lo saco de la cola (es el unico Pc en ella)
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

            if(runningProccessNode->proccess.state == KILLED){

                if(runningProccessNode->proccess.awakeParent)
                    unblock(runningProccessNode->proccess.pPid);
                
                removeProccess(runningProccessNode); 

            } else
                processEnqueue(&activeProccesses, runningProccessNode);
        }
    }

    if(activeProccesses.readyCount > 0){

        runningProccessNode = processDequeue(&activeProccesses);
        while(runningProccessNode->proccess.state != READY){

            if(runningProccessNode->proccess.state == KILLED)
                removeProccess(runningProccessNode);
            else if(runningProccessNode->proccess.state == BLOCKED) //PVS indica que la condicion es siempre verdadera pero nos parece mejor dejarlo por claridad
                processEnqueue(&activeProccesses, runningProccessNode);
            
            runningProccessNode = processDequeue(&activeProccesses);
        }
    }else
        runningProccessNode = dummyProcessNode;

    runtimeLeft = runningProccessNode->proccess.priority + 1; //Heuristica

    return runningProccessNode->proccess.rsp;
}

uint64_t initializeProccess(void (*function)(int , char **), uint8_t fg, int argc, char ** argv, uint16_t * stdFd){

    // Solo un proceso foreground puede crear otro
    if(fg && !runningProccessNode->proccess.fg)
        return 0;

    proccessNode * node = malloc2(PROCCESS_STACK_SIZE + sizeof(proccessNode));

    if(node == NULL)
        return 0;

    argv = copyArguments((char **)(((uint64_t)node) + sizeof(proccessNode)), argc, argv);
    createProccess(node, argv[0], fg, stdFd);
    processEnqueue(&activeProccesses, node);

    stackFrame newSF;
    newSF.ss = 0;
    newSF.rsp = node->proccess.rbp;
    newSF.rflags = 0x202;
    newSF.cs = 0x8;
    newSF.rip = (uint64_t) loader2;
    newSF.rdi = argc - 1;
    newSF.rsi = (uint64_t) ++argv;
    newSF.rdx = (uint64_t) function;
    newSF.rcx = node->proccess.pid;

    memcpy((void *)(node->proccess.rsp), &newSF, sizeof(stackFrame));

    if(runningProccessNode != NULL && fg){
        block(runningProccessNode->proccess.pid);
        return runningProccessNode->proccess.pid; // Ya el valor de retorno no tiene sentido
    }

    return node->proccess.pid;
}

static char ** copyArguments(char ** newArgv, int argc, char ** argv){
    char * dest = (char*)(newArgv + argc);

    for(uint16_t i = 0; i < argc; i++){
        newArgv[i] = dest;
        for(char* aux = argv[i]; *aux; aux++, dest++){
            *dest = *aux;
        }
        *dest = 0;
        dest++;
    }
    
    return newArgv;
}

static void createProccess(proccessNode * node, char* name, uint8_t fg, uint16_t * stdFd){
    proccess_t * p = &node->proccess;

    p->rbp = (uint64_t)node + PROCCESS_STACK_SIZE + sizeof(proccessNode) - sizeof(uint64_t);
    p->rsp = (uint64_t)(node->proccess.rbp - sizeof(stackFrame));
    p->name = name;
    p->pid = getNewPID();
    p->fg = fg;
    p->priority = (fg)? DEFAULT_FOREGROUND_PRIORITY : DEFAULT_BACKGROUND_PRIORITY;
    p->state = READY;
    p->stdIn = (stdFd)? stdFd[0] : 0;
    p->stdOut = (stdFd)? stdFd[1] : 0;
    p->pPid = (runningProccessNode)? runningProccessNode->proccess.pid : 0;
    p->awakeParent = (runningProccessNode && fg)? 1 : 0;
}

static uint64_t getNewPID(){
    pidCounter++;
    return pidCounter - 1;
}

static void removeProccess(proccessNode * node){
    free2(node);
}

static void processEnqueue(proccessNodeQueue * q, proccessNode * node){
    if(q == NULL || node == NULL)
        return;

    if(q->first == NULL)
        q->first = node;
    else
        q->last->next = node;

    q->last = node;
    node->next = NULL;

    if(node->proccess.state == READY)
        activeProccesses.readyCount++;
}

static proccessNode* processDequeue(proccessNodeQueue * q){
    if(q == NULL || isProcessQueueEmpty(q))
        return NULL;

    proccessNode* ans = q->first;

    if(q->last == ans)
        q->last = NULL;

    q->first = q->first->next;

    if(ans->proccess.state == READY)
        activeProccesses.readyCount--;

    return ans;
}

static int isProcessQueueEmpty(proccessNodeQueue * q){
    return q == NULL || q->first == NULL;
}

static proccessNode * getProccessNodeFromPID(uint64_t pid){
        if(runningProccessNode->proccess.pid == pid)
            return runningProccessNode;
        
        if(dummyProcessNode->proccess.pid == pid)
            return dummyProcessNode;

        for(proccessNode* iter = activeProccesses.first; iter != NULL; iter = iter->next){ //Lo busco en la cola
            if(iter->proccess.pid == pid)
                return iter;
        }

    return NULL;
}

static void loader2(int argc, char *argv[], void (*function)(int , char **)){
    function(argc, argv);
    exit();
}

static int changeProccessState(uint64_t pid, enum states state){
    proccessNode * node = getProccessNodeFromPID(pid);

    if(node == NULL || node == dummyProcessNode || node->proccess.state == KILLED) //No hay proceso asociado a pid
        return -1;

    if(node == runningProccessNode){
        if(runningProccessNode->proccess.state == state)
            return 1;

        runningProccessNode->proccess.state = state;
        return 0;
    }

    if(node->proccess.state == state)
        return 1;

    if(node->proccess.state != READY && state == READY)
        activeProccesses.readyCount++;

    else if(node->proccess.state == READY && state != READY)
        activeProccesses.readyCount--;

    node->proccess.state = state;

    return 0;
}

void changeProccessPriority(uint64_t pid, uint8_t priority){
    if(priority > MAX_PRIORITY)
        priority = MAX_PRIORITY;

    proccessNode* node = getProccessNodeFromPID(pid);

    if(node == NULL || node == dummyProcessNode) //No hay process asociado a pid
        return;

    node->proccess.priority = priority;
}

void exit(){
    kill(runningProccessNode->proccess.pid);
}

int kill(uint64_t pid){ 
    int ans = changeProccessState(pid, KILLED);

    if(pid == runningProccessNode->proccess.pid) // Es lo mismo que exit
        callTimerTick();

    return ans;
}

int block(uint64_t pid){
    int ans = changeProccessState(pid, BLOCKED);

    if(pid == runningProccessNode->proccess.pid) // Espero a que el scheduler me saque
        callTimerTick();

    return ans;
}

int unblock(uint64_t pid){
    return changeProccessState(pid, READY);
}

void skipTurn(){
    runtimeLeft = 0;
    callTimerTick();
}

uint64_t getPID(){
    return runningProccessNode->proccess.pid;
}

uint16_t getRunningProcessStdIn(){
    return runningProccessNode->proccess.stdIn;
}

uint16_t getRunningProcessStdOut(){
    return runningProccessNode->proccess.stdOut;
}

uint8_t amIFg(){
    return runningProccessNode->proccess.fg;
}

void wait(uint64_t pid){
    proccessNode * child;
    
    if((child = getProccessNodeFromPID(pid)) != NULL){
        child->proccess.awakeParent = 1;
        block(runningProccessNode->proccess.pid);
    } 
}

static void dummyFunction(int argc, char ** argv){
    while(1){
        //println("Dummy");
        _hlt();
    }
}

void dumpScheduler(){
    uint64_t totalP = 1; //El running proccess

    if(runningProccessNode != NULL){
        println("Current Running Proccess:");
        dumpProccess(runningProccessNode->proccess);
        printString("Runetime Left (in Ticks): ");
        printint(runtimeLeft);
        putchar('\n');
    }

    println("Active Processes:");
    totalP += dumpQueueProcesses(&activeProccesses);

    printString("Total Proccesses in System: ");
    printint(totalP);
    putchar('\n');

    printString("Total Proccesses Created: ");
    printint(pidCounter - 2); //No contamos dummy
    putchar('\n');
}

void dumpProcessFromPID(uint64_t pid){
    proccessNode* node = getProccessNodeFromPID(pid);
    if(node == NULL)
        return;

    dumpProccess(node->proccess);
}

static void dumpProccess(proccess_t p){
    printString("Name: "); printString(p.name);

    printString(" PID: "); printint(p.pid);

    printString(" RSP: 0x"); printhex(p.rsp);

    printString(" RBP: 0x"); printhex(p.rbp);

    (p.fg)? printString(" Foreground ") : printString(" Background ");

    printString("Priority: "); printint(p.priority);

    printString(" State: ");
    switch(p.state){
        case READY:
            printString("READY"); break;
        case KILLED:
            printString("KILLED"); break;
        case BLOCKED:
            printString("BLOCKED"); break;
    }

    printString(" StdIn: "); printint(p.stdIn);
    printString(" StdOut: "); printint(p.stdOut);

    putchar('\n');
}

static uint64_t dumpQueueProcesses(proccessNodeQueue * q){
    uint64_t count = 0;

    for(proccessNode* iter = q->first; iter != NULL; iter = iter->next){
        putchar('\t');
        dumpProccess(iter->proccess);
        count++;
    }
    return count;
}