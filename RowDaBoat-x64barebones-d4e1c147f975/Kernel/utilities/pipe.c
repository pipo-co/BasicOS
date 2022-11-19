// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include <pipe.h>
#include <sem.h>
#include <lib.h>
#include <screenDriver.h>

#define MAX_PIPE 50
#define MAX_NAME 50
#define PIPE_SIZE 1024

#define READ_PIPE "read_pipe"
#define WRITE_PIPE "write_pipe"

typedef struct pipe{
    char buffer[PIPE_SIZE];
    char name[MAX_NAME];
    uint16_t head;
    uint16_t tail;
    int32_t readSem;
    int32_t writeSem;
    uint16_t connectedCount;
    uint8_t active;
}pipe_t;

typedef struct pipes{
    pipe_t pipeArray[MAX_PIPE];
    uint16_t firstInactive;
    uint16_t size;
    int32_t creationMutex;
}pipes_t;

static pipes_t pipes;

static int isValidPipe(uint16_t pipeId);
static int initializePipe(char * pipeName);
static int32_t getPipeId(char * pipeName);
static void dumpPipe(pipe_t * pipe);
static void updateFirstInactive(uint16_t oldValue);

//Circular static queue
static void pipeEnqueue(pipe_t * pipeQueue, char c);
static char pipeDequeue(pipe_t * pipeQueue);


int initPipes(){
    if((pipes.creationMutex = createSem("mutex_pipes", 1)) == -1)
        return -1;
    return 0;
}

int32_t openPipe(char * pipeName){

    semWait(pipes.creationMutex);
    
    int32_t pipeId = getPipeId(pipeName);
    
    //Ya existe un pipe con ese nombre 
    if(pipeId != -1){
        pipes.pipeArray[pipeId].connectedCount++;
        semPost(pipes.creationMutex);
        return pipeId + 1;
    }

    if(pipes.size >= MAX_PIPE){
        semPost(pipes.creationMutex);
        return -1;  
    }

    pipeId = initializePipe(pipeName);

    semPost(pipes.creationMutex);

    if(pipeId == -1)
        return -1;

    return pipeId + 1;   
}

int writePipe(uint16_t pipeId, char c){
    if(!isValidPipe(pipeId))
        return -1;
    
    pipeId--;

    if(!pipes.pipeArray[pipeId].active)
        return -1;

    semWait(pipes.pipeArray[pipeId].writeSem);

    pipeEnqueue(&pipes.pipeArray[pipeId], c);

    semPost(pipes.pipeArray[pipeId].readSem);

    return 0;
}

char readPipe(uint16_t pipeId){
    if(!isValidPipe(pipeId))
        return 0;

    pipeId--;

    if(!pipes.pipeArray[pipeId].active)
        return 0;

    semWait(pipes.pipeArray[pipeId].readSem);

    char c = pipeDequeue(&pipes.pipeArray[pipeId]);

    semPost(pipes.pipeArray[pipeId].writeSem);

    return c;
}

void closePipe(uint16_t pipeId){
    if(!isValidPipe(pipeId))
        return;
        
    pipeId--;

    semWait(pipes.creationMutex);

    if(!pipes.pipeArray[pipeId].active){
        semPost(pipes.creationMutex);
        return;
    }

    pipes.pipeArray[pipeId].connectedCount--;
    if(pipes.pipeArray[pipeId].connectedCount > 0){
        semPost(pipes.creationMutex);
        return;
    }

    removeSem(pipes.pipeArray[pipeId].readSem);
    removeSem(pipes.pipeArray[pipeId].writeSem);

    if(pipes.firstInactive > pipeId)
        pipes.firstInactive = pipeId;

    pipes.pipeArray[pipeId].active = 0;
    pipes.size--;
    

    semPost(pipes.creationMutex);
}

int writeStringPipe(uint16_t pipeId, char * s){
    if(!isValidPipe(pipeId) || !pipes.pipeArray[pipeId - 1].active)
        return -1;

    while(*s)
        writePipe(pipeId, *s++);
    
    return 0;
}

void dumpPipes(){
    printString("Number of pipes active: ");printint(pipes.size);putchar('\n');
    for(int i = 0; i < MAX_PIPE; i++){
        if(pipes.pipeArray[i].active){
            printString("Code: "); printint(i + 1); putchar(' ');
            dumpPipe(&pipes.pipeArray[i]);
        }
    }
}

static int isValidPipe(uint16_t pipeId){
    return pipeId > 0 && pipeId <= MAX_PIPE;
} 

static int initializePipe(char * pipeName){
    uint16_t pipeId = pipes.firstInactive;
    char buffer[20];
    
    //Iniciar los semaforos de lectura y escritura
    uintToBase(pipeId, buffer, 10);
    strcat(buffer, READ_PIPE);
    if((pipes.pipeArray[pipeId].readSem = createSem(buffer, 0)) == -1 )
        return -1;
    
    uintToBase(pipeId, buffer, 10);
    strcat(buffer, WRITE_PIPE);
    if((pipes.pipeArray[pipeId].writeSem = createSem(buffer, PIPE_SIZE)) == -1)   
        return -1;
  
    pipes.pipeArray[pipeId].head = 0;
    pipes.pipeArray[pipeId].tail = 0;
    pipes.pipeArray[pipeId].active = 1;
    pipes.pipeArray[pipeId].connectedCount = 1;
    strncpy(pipes.pipeArray[pipeId].name, pipeName, MAX_NAME - 1);

    pipes.size++;

    updateFirstInactive(pipeId);
    
    return pipeId;
}

static void updateFirstInactive(uint16_t oldValue){
    for(int i = oldValue + 1; i < MAX_PIPE; i++){
        if(!pipes.pipeArray[i].active){
            pipes.firstInactive = i;
            return;
        }
    }
    pipes.firstInactive = MAX_PIPE;
}

static void dumpPipe(pipe_t * pipe){
    printString("Name: "); printString(pipe->name);

    printString(" Head: 0x"); printhex(pipe->head);

    printString(" Tail: 0x"); printhex(pipe->tail);
    
    printString(" Processes Connected to Pipe: "); printint(pipe->connectedCount);

    printString(" ReadSemCode: "); printint(pipe->readSem);

    printString(" WriteSemCode: "); printint(pipe->writeSem); putchar('\n');

    printString("Content: "); 
    for(int i = pipe->head; i != pipe->tail; i = i + 1 % PIPE_SIZE)
        putchar(pipe->buffer[i]);
    putchar('\n');

    println("Processes Blocked by Pipe: ");

    dumpProcessesBlockedBySem(pipe->readSem);
    dumpProcessesBlockedBySem(pipe->writeSem);
}


static int32_t getPipeId(char * name){
    int i = 0;

    for(uint16_t activePipeCount = 0; activePipeCount < pipes.size; i++){
        if(pipes.pipeArray[i].active){
            if(strcmp(name, pipes.pipeArray[i].name)) //a diferencia de strcmp de string.h, strcmp devuelve !=0 si los strings son iguales
                return i;
            activePipeCount++;
        }
    }
    return -1;
}


//Queue methods. Por los semaforos no se necesitan validaciones
static void pipeEnqueue(pipe_t * pipeQueue, char c){
    
    pipeQueue->buffer[pipeQueue->tail] = c;
        
    pipeQueue->tail = (pipeQueue->tail + 1) % PIPE_SIZE;
}

static char pipeDequeue(pipe_t * pipeQueue){
    
    char ans = pipeQueue->buffer[pipeQueue->head];
    
    pipeQueue->head = (pipeQueue->head + 1) % PIPE_SIZE;

    return ans;
}