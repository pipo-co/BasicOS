#include <stdint.h>
#include <scheduler.h>
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define PRIORITY_COUNT 5
#define TIME_MULT 2
#define PROCCESS_STACK_SIZE (8 * 1024) //8 KiB
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
}queueHeader;

static queueHeader activeProccessesInitializer[PRIORITY_COUNT];
static queueHeader expiredProccessesInitializer[PRIORITY_COUNT];
static queueHeader * activeProccesses = activeProccessesInitializer;
static queueHeader * expiredProccesses = expiredProccessesInitializer;

static proccessNode * runningProccessNode;
static uint64_t runetimeLeft;
static uint16_t pidCounter = 1;

static void removeProccess(proccessNode * node);
static int isEmpty(queueHeader * q);
static void push(queueHeader * q, proccessNode * node);
static proccessNode* pop(queueHeader * q);
static proccessNode * getProccessNodeFromPID(uint16_t pid);
static uint64_t swapProccess(uint64_t rsp);
static void changeProccessPriority(uint16_t pid, uint8_t prority);
static void changeProccessState(uint16_t pid, enum states state);
static proccessNode * getAndRemoveProccessNodeFromPID(uint16_t pid);
static void createProccess(proccessNode * node, char* name, uint8_t fg, uint8_t prority);
static uint16_t getNewPID();

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

uint64_t scheduler(uint64_t rsp){
    if(runetimeLeft > 0){
        runetimeLeft--;
        return rsp;
    }
    return swapProccess(rsp);
}

static uint64_t swapProccess(uint64_t rsp){
    runningProccessNode->proccess.rsp = rsp;

    uint8_t currentPriority = runningProccessNode->proccess.priority;
    
    if(runningProccessNode->proccess.state == KILLED)
        removeProccess(runningProccessNode);
    else
        push(&expiredProccesses[currentPriority], runningProccessNode);

    while(currentPriority < PRIORITY_COUNT && isEmpty(&activeProccesses[currentPriority]))
        currentPriority++;

    if(currentPriority == PRIORITY_COUNT){
        queueHeader * aux = activeProccesses;
        activeProccesses = expiredProccesses;
        expiredProccesses = aux;
        currentPriority = 0;
    }

    runningProccessNode = pop(&activeProccesses[currentPriority]);

    runetimeLeft = (PRIORITY_COUNT - currentPriority) * TIME_MULT;

    return runningProccessNode->proccess.rsp;
}

uint16_t initializeProccess(int (*function)(int , char **), char* name, uint8_t fg, int argc, char ** argv){

    proccessNode * node = malloc2(PROCCESS_STACK_SIZE + sizeof(proccessNode));
    if(node == NULL)
        return 0;

    createProccess(node, name, fg, DEFAULT_PRIORITY);
    push(&expiredProccesses[node->proccess.priority], node);

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

    memcpy((void *)(node->proccess.rbp - sizeof(stackFrame)), &newSF, sizeof(stackFrame));

    return node->proccess.pid;
}

static void createProccess(proccessNode * node, char* name, uint8_t fg, uint8_t prority){
    proccess_t p = node->proccess;

    p.rbp = (uint64_t)node + PROCCESS_STACK_SIZE + sizeof(proccessNode) - 1;
    p.rsp = p.rbp; //Not neccesary
    p.name = name;
    p.pid = getNewPID();
    p.fg = fg;
    p.priority = prority;
    p.state = READY;
}

static uint16_t getNewPID(){
    pidCounter++;
    return pidCounter - 1;
}

static void removeProccess(proccessNode * node){ //Faltan cosas capaz
    free2(node);
}

static void push(queueHeader * q, proccessNode * node){
    if(q == NULL || node == NULL)
        return;

    if(q->first == NULL)
        q->first = node;
    else
        q->last->next = node;

    q->last = node;
    node->next = NULL;
}

static proccessNode* pop(queueHeader * q){
    if(q == NULL || isEmpty(q))
        return NULL;

    proccessNode* ans = q->first;

    if(q->last == ans)
        q->last = NULL;

    q->first = q->first->next;

    return ans;
}

static int isEmpty(queueHeader * q){
    if(q == NULL)
        return 1;
    return q->first == NULL;
}

static proccessNode * getProccessNodeFromPID(uint16_t pid){
    for(uint8_t i = 0; i < PRIORITY_COUNT; i++){
        for(proccessNode* iter = activeProccesses[i].first; iter != NULL; iter = iter->next){
            if(iter->proccess.pid == pid)
                return iter;
        }
        for(proccessNode* iter = expiredProccesses[i].first; iter != NULL; iter = iter->next){
            if(iter->proccess.pid == pid)
                return iter;
        }
    }
    return NULL;
}

void loader2(int argc, char *argv[], int (*function)(int , char **)){
    function(argc, argv);
    exit();
}

static void changeProccessState(uint16_t pid, enum states state){
    if(runningProccessNode->proccess.pid == pid){
        runningProccessNode->proccess.state = state;
        return;
    }

    proccessNode * node = getProccessNodeFromPID(pid);
    if(node != NULL)
        node->proccess.state = state;
}

static void changeProccessPriority(uint16_t pid, uint8_t priority){
    proccessNode* node;
    if(runningProccessNode->proccess.pid == pid)
        node = runningProccessNode;
    else 
        node = getAndRemoveProccessNodeFromPID(pid);

    if(node == NULL)
        return;

    node->proccess.priority = priority;
    push(&expiredProccesses[priority], node);  
}

static proccessNode * getAndRemoveProccessNodeFromPID(uint16_t pid){ //Muy feo
    for(uint8_t i = 0; i < PRIORITY_COUNT; i++){
        for(proccessNode* iter = activeProccesses[i].first, *prev = iter; iter != NULL; prev = iter, iter = iter->next){
            if(iter->proccess.pid == pid){
                if(prev == iter){
                    activeProccesses[i].first = iter->next;
                    if(iter->next == NULL)
                        activeProccesses[i].last = NULL;
                } else {
                    prev->next = iter->next;
                    if(iter->next == NULL)
                        activeProccesses[i].last = prev;
                }
            return iter;  
            }
        }
        for(proccessNode* iter = expiredProccesses[i].first, *prev = iter; iter != NULL; prev = iter, iter = iter->next){
            if(iter->proccess.pid == pid){
                if(prev == iter){
                    expiredProccesses[i].first = iter->next;
                    if(iter->next == NULL)
                        expiredProccesses[i].last = NULL;
                } else {
                    prev->next = iter->next;
                    if(iter->next == NULL)
                        expiredProccesses[i].last = prev;
                }
            return iter;  
            }
        }
    }
    return NULL;
}

void exit(){
    kill(runningProccessNode->proccess.pid);
    _hlt();
}

void kill(uint16_t pid){ 
    changeProccessState(pid, KILLED);
}

uint16_t getPID(){
    return runningProccessNode->proccess.pid;
}

void dumpScheduler(){
    uint16_t totalP = 1; //El running proccess

    println("Current Running Proccess:");
    dumpProccess(runningProccessNode->proccess);
    printString("Runetime Left (in Ticks): ");
    printint(runetimeLeft);
    putchar('\n');

    println("Active Proccesses:");
    for(uint8_t i = 0; i < PRIORITY_COUNT; i++){
        for(proccessNode* iter = activeProccesses[i].first; iter != NULL; iter = iter->next){
            dumpProccess(iter->proccess);
            totalP++;
        }
    }

    println("Expired Proccesses:");
    for(uint8_t i = 0; i < PRIORITY_COUNT; i++){
        for(proccessNode* iter = expiredProccesses[i].first; iter != NULL; iter = iter->next){
            dumpProccess(iter->proccess);
            totalP++;
        }
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

