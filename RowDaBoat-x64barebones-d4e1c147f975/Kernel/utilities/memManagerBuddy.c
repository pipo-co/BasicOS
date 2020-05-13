
#ifdef BUDDY

#include <memoryManager.h>
#include <screenDriver.h>
#include <lib.h>
#include <sem.h>

enum powersOfTwo{KILO = 10, MEGA = 20, GIGA = 30};
#define MIN_POWER 6 //Min size 64 bytes (politica)
#define MAX_BUCKET_COUNT 30 - MIN_POWER //maximo que maneja el MM es 1 GiB (politica arbitraria)

#define BINARY_POWER(x) (1 << (x))

typedef struct list_t {
    uint8_t isFree;
    uint8_t level;
    struct list_t *prev, *next;
}list_t;

// Doubly linked circuar list:
// https://github.com/evanw/buddy-malloc/blob/master/buddy-malloc.c
static void list_init(list_t *list);
static void list_push(list_t *list, list_t *entry);
static void list_remove(list_t *entry);
static list_t *list_pop(list_t *list);

static int isListEmpty(list_t *list);
static void createAndPushNode(list_t* list, list_t* node , uint8_t level);

static uint8_t getBucket(uint32_t request);
static int getFirstAvBucket(uint8_t minBucket);
static list_t * getBuddyAddress(list_t *node);
static list_t * getPrincipalAdress(list_t *node);
static void insertNodeAndJoinSpace(list_t* node);
static void printBlockSize(uint8_t exp);
uint8_t ispowerof2(uint32_t x) ;

//Global variables
static list_t *heap_base;
static uint32_t heap_size;
static list_t listArray[MAX_BUCKET_COUNT];
static uint32_t availableMemory;
static uint8_t bucketCount;

static int16_t mutex;

int initMM(void * heap_baseInit, uint32_t heap_sizeInit){
    mutex = createSem("mutex_MM", 1);
    if(mutex == -1)
        return -1;

    heap_base = heap_baseInit;
    heap_size = availableMemory = heap_sizeInit;
    bucketCount = intLog2(heap_size) - MIN_POWER + 1;

    //Heap size demasiado chico
    if(bucketCount < 1) 
        return -1;  
    
    if(bucketCount > MAX_BUCKET_COUNT) //Si el espacio es mas grande que 1 GiB, lo truncamos
        bucketCount = MAX_BUCKET_COUNT;

    for(size_t i = 0; i < bucketCount; i++){
        list_init(&listArray[i]);
        listArray[i].isFree = 0;
        listArray[i].level = i;
    }
    
    createAndPushNode(&listArray[bucketCount - 1], heap_base, bucketCount - 1);

    return 0;
}

void * malloc2(unsigned bytes){
    if(bytes == 0)
        return NULL;

    bytes += sizeof(list_t);

    if(bytes > heap_size)
        return NULL;

    uint8_t bucket = getBucket(bytes);

    semWait(mutex);

    int parentBucket = getFirstAvBucket(bucket);
    if(parentBucket == -1){
        semPost(mutex);
        return NULL;
    }

    list_t *ptr;
    for(ptr = list_pop(&listArray[parentBucket]); bucket < parentBucket ; parentBucket--){
        ptr->level--;
        createAndPushNode(&listArray[parentBucket - 1], getBuddyAddress(ptr), parentBucket - 1);
    }
    ptr->isFree = 0;

    semPost(mutex);

    availableMemory -= BINARY_POWER(bucket + MIN_POWER);
   
    return (void *)(ptr + 1);
}

int free2(void * ap){
    if(ap == NULL)
        return 1;

    list_t* bp = (list_t*)ap - 1;

    semWait(mutex);
    
    bp->isFree = 1;

    availableMemory += BINARY_POWER(bp->level + MIN_POWER);
    
    insertNodeAndJoinSpace(bp);

    semPost(mutex);

    return 0;
}

uint32_t getAvailableMemory(){
    //El espacio libre no es el espacio que puede pdir el usuario. Habria que desconatr el tamaño que ocupan los headers nenecsarios para aprovecharlo
    //Se podria restar sizeof(list_t) cada vez que se agrega una entrada a listArray y sumar la misma cantidad cuando se junta con su buddy
    return availableMemory;
}

void dumpMM(){
    list_t *dummy, *iter;
    uint32_t nodeCount = 0;
    uint32_t totalFreeSpace = 0;

    for(int i = bucketCount - 1; i >= 0; i--){
        dummy = &listArray[i];

        if(!isListEmpty(dummy)){
            printString("Available blocks of size: ");
            printBlockSize(i + MIN_POWER);

            for(iter = dummy->next, nodeCount = 0; iter != dummy; nodeCount++, iter = iter->next){
                printString("Node number: ");
                printint(nodeCount);
                printString(". Pointer: 0x");
                printhex((uintptr_t) iter);
                printString(". Next: 0x");
                printhex((uintptr_t) iter->next);
                printString(". Prev: 0x");
                printhex((uintptr_t) iter->prev);
                printString(". Level: ");
                printint((uintptr_t) iter->level);
                if(iter->isFree)
                    println(" Block is free.");
                else
                    println(" Block is't free.");
            }
            putchar('\n');

            totalFreeSpace += nodeCount * BINARY_POWER(i + MIN_POWER);  
        }
    }
    printString("Total Free Space: ");
    printint(totalFreeSpace);
    println(" B.");
}

static void printBlockSize(uint8_t exp){
    char * unidad;
    if(exp < KILO){
        unidad = " B.";
    } else if ( exp < MEGA) {
        unidad = " KB.";
        exp -= KILO;
    } else if (exp < GIGA){
        unidad = " MB.";
        exp -= MEGA;
    } else {
        unidad = " GB.";
        exp -= GIGA;
    }
    printint(1 << exp);
    println(unidad);
}

//doubly linked circular list fuctions
//https://github.com/evanw/buddy-malloc/blob/master/buddy-malloc.c

static void list_init(list_t *list){
    list->prev = list;
    list->next = list;
}

static void list_push(list_t *list, list_t *entry){
    list_t *prev = list->prev;
    entry->prev = prev;
    entry->next = list;
    prev->next = entry;
    list->prev = entry;
}

static void list_remove(list_t *entry){
    list_t *prev = entry->prev;
    list_t *next = entry->next;
    prev->next = next;
    next->prev = prev;
}

static list_t *list_pop(list_t *list){
    list_t *back = list->prev;
    if (back == list) return NULL;
    list_remove(back);
    return back;
}

static int isListEmpty(list_t *list){
    return list->prev == list;
}

static void createAndPushNode(list_t* list, list_t* node , uint8_t level){
    node->isFree = 1;
    node->level = level;
    list_push(list, node);
}

static void insertNodeAndJoinSpace(list_t* node){
    list_t *buddy = getBuddyAddress(node);

    while(node->level != bucketCount - 1 && buddy->level == node->level && buddy->isFree){
        list_remove(buddy);

        node = getPrincipalAdress(node);
        node->level++;

        buddy = getBuddyAddress(node);
    }
    list_push(&listArray[node->level], node);
}

static list_t * getBuddyAddress(list_t *node){
    uint8_t level = node->level;
    uintptr_t currentOffset = (uintptr_t)node - (uintptr_t)heap_base;
    uintptr_t newOffset = currentOffset ^ BINARY_POWER(MIN_POWER + level);

    return (list_t*)(newOffset + (uintptr_t)heap_base);
}

static list_t * getPrincipalAdress(list_t *node){
    uint8_t level = node->level;
    uintptr_t mask = BINARY_POWER(MIN_POWER + level);
    mask = ~mask; 

    uintptr_t currentOffset = (uintptr_t)node - (uintptr_t)heap_base;
    uintptr_t newOffset = currentOffset & mask;  

    return (list_t*)(newOffset + (uintptr_t)heap_base);
}

static uint8_t getBucket(uint32_t request){

    uint8_t aux = intLog2(request);
    if(aux < MIN_POWER)
        return 0;

    aux -= MIN_POWER;
    if(ispowerof2(request))
        return aux;

    return aux + 1;
}

static int getFirstAvBucket(uint8_t minBucket){
    for(; minBucket < bucketCount && isListEmpty(&listArray[minBucket]); minBucket++);

    return (minBucket < bucketCount)? minBucket : -1;
}

//https://stackoverflow.com/questions/3638431/determine-if-an-int-is-a-power-of-2-or-not-in-a-single-line
uint8_t ispowerof2(uint32_t x) {
   return x && !(x & (x - 1));
}
#endif