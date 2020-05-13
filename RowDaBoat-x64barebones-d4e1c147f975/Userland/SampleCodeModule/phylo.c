#include <stdint.h>
#include <usrlib.h>
#include <stddef.h>

#define MAX_PHIL 40
#define MIN_PHIL 2
#define SEM_NAME "_phyl"
#define NAME_LEN 20
#define EOF 27  // ESC
#define PRINT_THINKING '.'
#define PRINT_EATING 'E'
#define TIME_MULT 2

#define LEFT_CHOPSTICK(p) (chopstick[p])
#define RIGHT_CHOPSTICK(p) (chopstick[(p + 1) % phyloCount])

enum state {HUNGRY, EATING, THINKING};

typedef struct phylo{
    uint64_t pid;
    enum state state;
}phylo_t;


static int evenPhylo(int argc, char ** argv);
static int oddPhylo(int argc, char ** argv);
static void createPhylosopher();
static void removePhylosopher();
static void init(uint16_t initPhyloCount);
static void guaranteeThinkingPhylo(uint16_t phylo);
static void updateAndPrintTableState(uint16_t phyloId, enum state newState);
static void freeResources();

static void takeRightChop(uint16_t phyloID);
static void takeLeftChop(uint16_t phyloID);
static void releaseRightChop(uint16_t phyloID);
static void releaseLeftChop(uint16_t phyloID);


static sem_t chopstick[MAX_PHIL];

static phylo_t phylosophers[MAX_PHIL];
static uint16_t phyloCount;

static char tableState[MAX_PHIL + 1];

void phylo(int argcount, char * args[]){
    uint16_t initPhyloCount = MIN_PHIL;

    if(argcount > 0)
        initPhyloCount = atoi(args[0]);

    init(initPhyloCount);
    
    char c;
    while((c = getChar()) != EOF){
        switch(c){
        case 'a':
            createPhylosopher();
            break;
        case 'r':
            removePhylosopher();
            break;
        }
    }
    freeResources();
}

static int evenPhylo(int argc, char ** argv){
    
    uint16_t phyloID = atoi(argv[0]);
    if(phyloID % 2 != 0)
        return 1;

    while(1){
        // Matamos HUNGRY??
        phylosophers[phyloID].state = HUNGRY;
        takeLeftChop(phyloID);
        takeRightChop(phyloID);

        phylosophers[phyloID].state = EATING;
        updateAndPrintTableState(phyloID, EATING);
        sleep(1 * TIME_MULT);

        // Cambiar orden??
        updateAndPrintTableState(phyloID, THINKING);
        releaseRightChop(phyloID);
        releaseLeftChop(phyloID);
        phylosophers[phyloID].state = THINKING;
        
        sleep(2 * TIME_MULT);
    }
    return 0;
}

static int oddPhylo(int argc, char ** argv){
    
    uint16_t phyloID = atoi(argv[0]);
    if(phyloID % 2 == 0)
        return 1;

    while(1){
        phylosophers[phyloID].state = HUNGRY;
        takeRightChop(phyloID);
        takeLeftChop(phyloID);

        phylosophers[phyloID].state = EATING;
        updateAndPrintTableState(phyloID, EATING);
        sleep(1 * TIME_MULT);

        updateAndPrintTableState(phyloID, THINKING);
        releaseLeftChop(phyloID);
        releaseRightChop(phyloID);
        phylosophers[phyloID].state = THINKING;
        
        sleep(2 * TIME_MULT);
    }
    return 0;
}   

static void createPhylosopher(){
    if(phyloCount >= MAX_PHIL)
        return;

    char phyloName[NAME_LEN];
    char phyloID[NAME_LEN];
    uint16_t phylo = phyloCount;

    uintToBase(phylo, phyloName, 10);
    uintToBase(phylo, phyloID, 10);
    strcat(phyloName, SEM_NAME);

    phylosophers[phylo].state = THINKING;
    tableState[phylo] = PRINT_THINKING;
    
    guaranteeThinkingPhylo(phylo - 1);

    chopstick[phylo] = createSem(phyloName, 1);
    phyloCount++;

    unblock(phylosophers[phylo - 1].pid);

    char* argv[] = {phyloName, phyloID};
    phylosophers[phylo].pid = initializeProccess((phylo % 2 == 0)? evenPhylo : oddPhylo, 0, 2, argv, NULL);
}

static void removePhylosopher(){
   if(phyloCount <= MIN_PHIL)
        return;

    uint16_t phylo = phyloCount - 1;
    uint16_t phyloLeft = phylo - 1;
     
    guaranteeThinkingPhylo(phylo);
    guaranteeThinkingPhylo(phyloLeft);

    println("hola");
    kill(phylosophers[phylo].pid);
    tableState[phylo] = 0;

    phyloCount--;

    unblock(phylosophers[phyloLeft].pid);

    removeSem(chopstick[phylo]);
}

static void init(uint16_t initPhyloCount){
    char phyloName[NAME_LEN];
    char phyloID[NAME_LEN];

    if(initPhyloCount > MAX_PHIL)
        initPhyloCount = MAX_PHIL;

    else if(initPhyloCount < MIN_PHIL)
        initPhyloCount = MIN_PHIL;

    phyloCount = initPhyloCount;

    for(uint16_t i = 0; i < initPhyloCount; i++){
        uintToBase(i, phyloName, 10);
        strcat(phyloName, SEM_NAME);
        chopstick[i] = createSem(phyloName, 1);
        tableState[i] = PRINT_THINKING;
    }

    for(uint16_t i = 0; i < initPhyloCount; i++){
        
        uintToBase(i, phyloName, 10);
        strcat(phyloName, SEM_NAME);
        uintToBase(i, phyloID, 10);

        phylosophers[i].state = THINKING;

        char* argv[] = {phyloName, phyloID};
        phylosophers[i].pid = initializeProccess((i % 2 == 0)? evenPhylo : oddPhylo, 0, 2, argv, NULL);
    }
}

// Garantiza que el phylo especificado este bloqueado en el estado THINKING
static void guaranteeThinkingPhylo(uint16_t phylo){
    while(1){

        // Phylo ya esta bloqueado entonces seguro no esta THINKING
        if(block(phylosophers[phylo].pid) == 1)
            sleep(1);

        // Phylo esta usando los cubiertos
        else if(phylosophers[phylo].state != THINKING){
            unblock(phylosophers[phylo].pid);
            sleep(1);
        }   
        // Phylo esta pensando
        else
            return;
    }
}

static void freeResources(){
    for(uint16_t i = 0; i < phyloCount; i++)
        kill(phylosophers[i].pid);

    for(uint16_t i = 0; i < phyloCount; i++)
        removeSem(chopstick[i]);

    phyloCount = 0;

    for(uint16_t i = 0; tableState[i] != 0; i++)
        tableState[i] = 0;
}

static void updateAndPrintTableState(uint16_t phyloId, enum state newState){
    tableState[phyloId] = (newState == EATING)? PRINT_EATING : PRINT_THINKING;
    println(tableState);
}

static void takeRightChop(uint16_t phyloID){
    semWait(RIGHT_CHOPSTICK(phyloID));
}

static void takeLeftChop(uint16_t phyloID){
    semWait(LEFT_CHOPSTICK(phyloID));
}

static void releaseRightChop(uint16_t phyloID){
    semPost(RIGHT_CHOPSTICK(phyloID));
}

static void releaseLeftChop(uint16_t phyloID){
    semPost(LEFT_CHOPSTICK(phyloID));
}