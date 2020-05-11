#include <stdint.h>
#include <usrlib.h>
#include <stddef.h>

#define MAX_PHIL 50
#define MIN_PHIL 2
#define SEM_NAME "_phyl"
#define EOF 27  // ESC

#define LEFT_CHOPSTICK(p) (chopstick[p])
#define RIGHT_CHOPSTICK(p) (chopstick[p + 1 % phyloCount])

enum state {HUNGRY, EATING, THINKING};


typedef struct phylo{
    uint64_t pid;
    enum state state;
}phylo_t;

static sem_t chopstick[MAX_PHIL];

phylo_t phylosophers[MAX_PHIL];
uint16_t phyloCount;

void phylo(){
    init();

    char c;
    while( (c = getChar()) != EOF){
        switch (c){
        case 'a':
            createPhylosopher();
            break;
        case 'r':
            if(phyloCount> MIN_PHIL)
                removePhylosopher();
            break;
        }
    }
}

int evenPhylo(int argc, char ** argv){
    
    uint16_t phyloID = atoi(argv[1]);
    if(phyloID % 2 != 0)
        return 1;

    while(1){
        phylosophers[phyloID].state = HUNGRY;
        takeLeftChop(phyloID);
        takeRightChop(phyloID);
        phylosophers[phyloID].state = EATING;
        sleep(1);
        releaseRightChop(phyloID);
        releaseLeftChop(phyloID);
        phylosophers[phyloID].state = THINKING;
        sleep(2);
    }
    return 0;
}

int oddPhylo(int argc, char ** argv){

    uint16_t phyloID = atoi(argv[1]);
    if(phyloID % 2 == 0)
        return 1;
        
    while(1){
        takeRightChop(phyloID);
        takeLeftChop(phyloID);
        sleep(1);
        releaseLeftChop(phyloID);
        releaseRightChop(phyloID);
        sleep(2);
    }
    return 0;
}   



static void createPhylosopher(){
    char phyloName[20];
    char phyloID[20];
    uint16_t phylo = phyloCount;

    uintToBase(phylo, phyloName, 10);
    uintToBase(phylo, phyloID, 10);
    strcat(phyloName, SEM_NAME);
    
    chopstick[phylo] = createSem(phyloName, 1);

    //if()

    phylosophers[phylo].state = THINKING;

    char* argv[] = {phyloName, phyloID};
    phylosophers[phylo].pid = initializeProccess((phyloCount % 2 == 0)? evenPhylo : oddPhylo, 0, 2, argv, NULL);

    phyloCount++;
}

static void init(){
    phyloCount = 0;

    for(; phyloCount < MIN_PHIL;){
        createPhylosopher();
    }
}