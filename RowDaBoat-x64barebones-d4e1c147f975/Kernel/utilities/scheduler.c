#include <stdint.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define PRIORITY_COUNT 5
#define TIME_MULT 2
#define PROCCESS_STACK_SIZE (8 * 1024 - sizeof(proccessNode)) //8 KiB
#define DEFAULT_PRIORITY (PRIORITY_COUNT/2)

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
    uint16_t readyCount;
}proccessNodeQueue;

static proccessNodeQueue activeProccesses;

static proccessNode * runningProccessNode;
static uint64_t runtimeLeft;
static uint16_t pidCounter = 1;

static proccessNode * dummyProcessNode;

static void removeProccess(proccessNode * node);
static int isEmpty(proccessNodeQueue * q);
static void push(proccessNodeQueue * q, proccessNode * node);
static proccessNode* pop(proccessNodeQueue * q);
static proccessNode * getProccessNodeFromPID(uint16_t pid);
static uint64_t swapProccess(uint64_t rsp);
static void changeProccessState(uint16_t pid, enum states state);
static void createProccess(proccessNode * node, char* name, uint8_t fg, uint8_t prority);
static uint16_t getNewPID();
static int dummyFunction(int argc, char ** argv);
static int hasReadyProcesses(proccessNodeQueue * q);

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

void initScheduler(){
    //Initialize Dummy Process
    initializeProccess(dummyFunction, "Dummy Process", 0, 0, NULL); //Mete al dummy process en la cola
    dummyProcessNode = pop(&activeProccesses); //Lo saco de la cola (es el unico Pc en ella)
    dummyProcessNode->proccess.priority = PRIORITY_COUNT; //Para que su runtime sea 0
}

uint64_t scheduler(uint64_t rsp){
    if(runningProccessNode->proccess.state != KILLED && runtimeLeft > 0){
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
                push(&activeProccesses, runningProccessNode);
        }
    }

    if(hasReadyProcesses(&activeProccesses)){

        runningProccessNode = pop(&activeProccesses);
        while(runningProccessNode->proccess.state != READY){

            if(runningProccessNode->proccess.state == KILLED)
                removeProccess(runningProccessNode);
            else if(runningProccessNode->proccess.state == BLOCKED)
                push(&activeProccesses, runningProccessNode);
            
            runningProccessNode = pop(&activeProccesses);
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
    push(&activeProccesses, node);
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

    if(node->proccess.state == READY)
        q->readyCount++;
}

static proccessNode* pop(proccessNodeQueue * q){
    if(q == NULL || isEmpty(q))
        return NULL;

    proccessNode* ans = q->first;

    if(q->last == ans)
        q->last = NULL;

    q->first = q->first->next;

    if(ans->proccess.state == READY)
        q->readyCount--;

    return ans;
}

static int isEmpty(proccessNodeQueue * q){
    return q == NULL || q->first == NULL;
}

static int hasReadyProcesses(proccessNodeQueue * q){
    return q != NULL && q->readyCount > 0;
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
            activeProccesses.readyCount++;
        else if(node->proccess.state == READY && state != READY)
            activeProccesses.readyCount--;

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
    while(1)
        _hlt();
    return 0;
}

