// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#ifndef BUDDY
#include "memoryManager.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define HEAP_SIZE (1024 * 128) //128 MB 
#define BLOCK_SIZE sizeof(node)
#define HEAP_TOTAL_BLOCKS (HEAP_SIZE / BLOCK_SIZE)
#define HEAP_BASE (heapBase)  //0x600000

typedef uint16_t Align;
union node{
    struct{
        union node *ptr;
        unsigned size; 
    }s;
    Align x;
};
typedef union node node;

static void joinMem(node *left, node *right);
static void printList();
static void testNacho1();
static void testNacho2();
static void testFaus();
static void testTobi();

static node* first;
char heapBase[HEAP_SIZE];

void initMM(){
    first = HEAP_BASE; //Deberia ser HEAP_BASE
    first->s.ptr = NULL;
    first->s.size = HEAP_TOTAL_BLOCKS;
}

//Heavily inspired in C malloc
void * malloc2(unsigned bytes){
    if(bytes == 0)
        return NULL;

    node* p;
    node* prevp = first; 
    unsigned blocks = (bytes % BLOCK_SIZE == 0) ? bytes/BLOCK_SIZE + 1 : bytes/BLOCK_SIZE + 2;

    for(p = first; p != NULL; prevp = p, p = p->s.ptr){

        if(p->s.size >= blocks){
            if(p->s.size == blocks){
                if(p == first)
                    first = p->s.ptr;
                else
                    prevp->s.ptr = p->s.ptr;
            }else{
                p->s.size -= blocks;
                p += p->s.size;
                p->s.size = blocks;
            }
            return (void*)(p + 1);
        }
    }
    return NULL;
} 

int free2(void * ap){

    if(ap == NULL){
        printf("Free: Pointer is NULL\n");
        return 1;
    }

    node* bp = (node*)ap - 1;

    //Creo que estas validaciones ya estan bien
    if(bp < (node*)HEAP_BASE || bp >= (node*)(HEAP_BASE + HEAP_SIZE)){
        printf("Free: Pointer Out of Bounds\n");
        return 1;
    }

    if(((uintptr_t)bp - (uintptr_t)HEAP_BASE) % BLOCK_SIZE != 0){
        printf("Free: Pointer is not a multiple of node\n");
        return 2;
    }
    
    node* p;

    //Lo saque para que si hacen free de la misma cosa no se rompa.
    //bp->s.ptr = NULL;

    if(first == NULL){
        first = bp;
        first->s.ptr = NULL;
        return 0;
    }

    if(bp < first){   
        joinMem(bp, first);
        first = bp;
        return 0;
    }
    
    for(p = first; p->s.ptr != NULL && p != bp && !(p < bp && bp < p->s.ptr); p = p->s.ptr); //Ubico a p antes de donde iria bp

    //A la segunda condicion le saque el igual. La tercera no la entiendo, no deberia ser menor?
    if(p == bp || bp < p + p->s.size){
        printf("Free: Pointer Already Freed\n");
        if(bp < p + p->s.size)
        printf("toldya\n");
        return 3;
    }

    //Right Join
    joinMem(bp, p->s.ptr);

    //Left Join
    joinMem(p, bp);

    return 0;
}

static void joinMem(node *left, node *right){
    
    if(left == NULL || (right != NULL && left >= right)){
        printf("joinMem: Invalid Join\n");
        return;
    }

    if(left + left->s.size == right){
        left->s.size += right->s.size;
        left->s.ptr = right->s.ptr;
    }else
        left->s.ptr = right;
}

int main(int argc, char const *argv[]){
    
    initMM();

    testTobi();
    // testNacho1();
    // testNacho2();
    // testFaus();
    
    printList();
}

static void printList(){
    int count = 0;
    node* iter = first;
    size_t freeMemory = 0;
    printf("Free memory blocks list\n");

    while(iter != NULL){
        printf("Node number: %2d:. ", count++);
        printf("Start adress: %p. ", (void *)iter);
        printf("Block size: %6u. ", iter->s.size);
        freeMemory += iter->s.size;
        printf("Next block adress: %p.\n", (void *)iter->s.ptr);
        iter = iter->s.ptr;
    }
    printf("Total free memory: %zu\n",freeMemory);
    putchar('\n');
}

static void testNacho1(){
    printf("Test nacho 1\n");
    void * ptr;
    int size = 1;
    while ((ptr = malloc2(size)) != NULL){
        free2(ptr);
        size += 5;
    }
    assert(getAvailableMemory() == HEAP_SIZE);
}

static void testNacho2(){
    printf("Test nacho 2\n");
    void * ptr = malloc2(1);
    void * auxPtr;
    int size = 2;

    while ((auxPtr = malloc2(size)) != NULL){
        free2(ptr);
        ptr = auxPtr;
        size += 1000;
        printList();
    }
    free2(ptr);
    assert(getAvailableMemory() == HEAP_SIZE);
}

static void testFaus(){
    int * arr = malloc2(32);
    void * arrBackUp = (void *) arr;

    for (int i = 0; i < 4; arr++, i++){
        *arr=i;
    }
    
    free2(arrBackUp);

    int * arr2 = malloc2(19);
    void * arrBackUp2 = (void *) arr2;
    for (int j = 0; j < 3; arr2++, j++){
        *arr2=j;
    }

    int * arr3 = malloc2(75000);
    void * arrBackUp3 = (void *) arr3;
    for (size_t i = 0; i < 10; arr3++, i++){
        *arr3=i;
    }

    free2(arrBackUp3);
    
    int * arr4 = malloc2(75000);
    void * arrBackUp4 = (void *) arr4;
    for (size_t i = 0; i < 10; arr4++, i++){
        *arr4=i;
    }
    
    free2(arrBackUp2);

    free2(arrBackUp4);

    int * arr6 = malloc2(131055);
    if(arr6 != NULL){
        void * arrBackUp6 = (void *) arr6;
        for (int j = 0; j < 3; arr6++, j++){
            *arr6=j;
        }

        free2(arrBackUp6);
    }

    assert(getAvailableMemory() == HEAP_SIZE);
}

static void testTobi(){

    printList();
    int* var1 = malloc2(1000);
    int* var2 = malloc2(1000);
    int* var3 = malloc2(1000);
    int* var4 = malloc2(1000);
    int* var5 = malloc2(1000);
    int* var6 = malloc2(1000);
    int* var7 = malloc2(1000);
    int* var8 = malloc2(1000);
    int* var9 = malloc2(1000);
    int* var10 = malloc2(1000);

    printList();
    free2(var9);
    
    
    printList();
    free2(var7);

    
    printList();
    free2(var5);
    printList();

    
    free2((node*)var9 + 1);
    printList();

    free2(var3);
    printList();
    free2(var6);
    printList();
    free2(var10);
    printList();
    free2(var1);
    printList();
    free2(var4);
    printList();
    free2(var2);
    printList();
    free2(var8);
    printList();

    //free2(heapBase + (first->s.size - 4) * BLOCK_SIZE + 16);
    printf("hola\n");
    printList();
    assert(getAvailableMemory() == HEAP_SIZE);
}

uint32_t getAvailableMemory(){
    
    uint32_t freeBlocks = 0;

    //Primero guarda el size y despues itera. Hacia segmentation fault.
    for(node *ptr = first; ptr != NULL; freeBlocks += ptr->s.size, ptr = ptr->s.ptr);
    
    return freeBlocks * BLOCK_SIZE;
}
#endif
#ifdef BUDDY

#include "memoryManager.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#define HEAP_SIZE (1024 * 1024 * 128) //128 MB 
#define HEAP_BASE (heapBase)  //0x600000
#define MIN_POWER 6 //Min size 64 bytes
#define MIN_ALLOC 64 //2^MIN_POWER
#define MAX_POWER 27 //log2(HEAP_SIZE)
#define BUCKET_COUNT (MAX_POWER - MIN_POWER + 1)
#define BINARY_POWER(x) (1 << (x))

typedef struct list_t {
    uint8_t isFree;
    uint8_t level;
    struct list_t *prev, *next;
}list_t;

// Lista circular doblemente encadenada:
// https://github.com/evanw/buddy-malloc/blob/master/buddy-malloc.c
// list_t, list_init, list_push, list_remove, list_pop
static void list_init(list_t *list);
static void list_push(list_t *list, list_t *entry);
static void list_remove(list_t *entry);
static list_t *list_pop(list_t *list);

static int isListEmpty(list_t *list);
void createAndPushNode(list_t* list, list_t* node , uint8_t level);

static uint8_t getBucket(uint32_t request);
int getFirstAvBucket(uint8_t minBucket);
list_t * getBuddyAddress(list_t *node);
list_t * getPrincipalAdress(list_t *node);
void insertNodeAndJoinSpace(list_t* node);
//list_t *tryJoin(list_t *node);

void printList();

char heapBase[HEAP_SIZE];
static list_t listArray[BUCKET_COUNT];
uint32_t availableMemory = HEAP_SIZE;


int main(){
    initMM();

    printList();
    int * v1 = malloc2(50000);
    printList();
    int * v2 = malloc2(50000);
    printList();
    int * v3 = malloc2(50000);
    printList();
    int * v4 = malloc2(50000);
    printList();
    int * v5 = malloc2(50000);
    printList();
    int * v6 = malloc2(50000);
    printList();
    int * v7 = malloc2(33554000);
    printList();
    int * v8 = malloc2(33554000);
    printList();
    int * v9 = malloc2(33554000);
    printList();
    if(malloc2(33554000) == NULL)
        printf("YASS\n");
    if(malloc2(1000000000) == NULL)
        printf("YASS\n");
    int * v10 = malloc2(16777000);
    printList();
    if(malloc2(16777000) == NULL)
        printf("YASS\n");

    free2(v1);
    printList();
    free2(v3);
    printList();
    free2(v5);
    printList();
    free2(v2);
    printList();
    free2(v4);
    printList();
    free2(v7);
    printList();
    free2(v9);
    printList();
    free2(v10);
    printList();
    free2(v8);
    printList();
    //v5 = malloc2(500000);
    //printList();
    free2(v6);
    printList();
}

void initMM(){
    for(size_t i = 0; i < BUCKET_COUNT; i++){
        list_init(&listArray[i]);
        listArray[i].isFree = 0;
        listArray[i].level = i;
    }
    createAndPushNode(&listArray[BUCKET_COUNT - 1], (list_t*)HEAP_BASE, BUCKET_COUNT - 1);
}

void * malloc2(unsigned bytes){
    if(bytes == 0)
        return NULL;

    printf("hola\n");
    bytes += sizeof(list_t);
    printf("Bytes: %u\n", bytes);

    if(bytes > HEAP_SIZE)
        return NULL;

    uint8_t bucket = getBucket(bytes);
    printf("Bucket: %u\n", bucket);

    int parentBucket = getFirstAvBucket(bucket);
    if(parentBucket == -1)
        return NULL;

    printf("Parent Bucket: %d\n", parentBucket);

    list_t *ptr;
    printf("vamos a ver pop\n");
    ptr = list_pop(&listArray[parentBucket]);
    printf("ptr %p ptrLevel: %d\n", (void*)ptr, ptr->level);

    for(; parentBucket > bucket; parentBucket--){
        printf("BuddyAddress %p\n", (void*)getBuddyAddress(ptr));
        ptr->level--;
        createAndPushNode(&listArray[parentBucket - 1], getBuddyAddress(ptr), parentBucket - 1);
    }

    printf("ptr %p\n", (void*)ptr);

    ptr->isFree = 0;
    //ptr->level = bucket;

    availableMemory -= BINARY_POWER(bucket + MIN_POWER);

    return (void *)(ptr + 1);
}

int free2(void * ap){
    if(ap == NULL)
        return 0;

    list_t* bp = (list_t*)ap - 1;
    printf("Pointer: %p\n", (void*)bp);

    bp->isFree = 1;

    availableMemory += BINARY_POWER(bp->level + MIN_POWER);
    
    insertNodeAndJoinSpace(bp);

    return 0;
}

void insertNodeAndJoinSpace(list_t* node){ //Prueba
    list_t *buddy = getBuddyAddress(node);

    while(node->level != BUCKET_COUNT - 1 && buddy->level == node->level && buddy->isFree){

        printf("Node: %p Level: %d Buddy: %p Level: %d isFree: %d \n", (void*)node, node->level, (void*)buddy, buddy->level, buddy->isFree);

        list_remove(buddy);

        node = getPrincipalAdress(node);
        node->level++;

        buddy = getBuddyAddress(node);
    }
    printf("Node: %p Level: %d Buddy: %p Level: %d isFree: %d \n", (void*)node, node->level, (void*)buddy, buddy->level, buddy->isFree);

    list_push(&listArray[node->level], node);
}

uint32_t getAvailableMemory(){
    return availableMemory;
}

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

void createAndPushNode(list_t* list, list_t* node , uint8_t level){
    node->isFree = 1;
    node->level = level;
    list_push(list, node);
}

list_t * getBuddyAddress(list_t *node){ //Luego sacar los casteos a HEAP_BASE
    uint8_t level = node->level;
    uintptr_t currentOffset = (uintptr_t)node - (uintptr_t)HEAP_BASE;
    uintptr_t newOffset = currentOffset ^ BINARY_POWER(MIN_POWER + level);

    return (list_t*)(newOffset + (uintptr_t)HEAP_BASE);
}

list_t * getPrincipalAdress(list_t *node){ //Luego sacar los casteos a HEAP_BASE
    uint8_t level = node->level;
    uintptr_t mask = BINARY_POWER(MIN_POWER + level);
    mask = ~mask; 

    uintptr_t currentOffset = (uintptr_t)node - (uintptr_t)HEAP_BASE;
    uintptr_t newOffset = currentOffset & mask;  

    return (list_t*)(newOffset + (uintptr_t)HEAP_BASE);
}

static uint8_t getBucket(uint32_t request){
  uint8_t bucket = 0;
  request--;
  request >>= MIN_POWER;

  while (request){
    bucket++;
    request >>= 1;
  }

  return bucket;
}

int getFirstAvBucket(uint8_t minBucket){
    for(; minBucket < BUCKET_COUNT && isListEmpty(&listArray[minBucket]); minBucket++);

    return (minBucket < BUCKET_COUNT)? minBucket : -1;
}

void printList(){
    list_t *dummy, *iter;
    uint32_t nodeCount = 0;
    uint32_t totalFreeSpace = 0;

    for(int i = BUCKET_COUNT - 1; i >= 0; i--){
        dummy = &listArray[i];

        if(!isListEmpty(dummy)){
            printf("Level: %d\n", i + MIN_POWER);

            for(iter = dummy->next, nodeCount = 0; iter != dummy; nodeCount++, iter = iter->next)
                printf("Node Number: %d Pointer: %p Next: %p Prev: %p Level %d Free: %d\n", nodeCount, (void*)iter, (void*)iter->next, (void*)iter->prev, iter->level, iter->isFree);
            printf("\n");

            totalFreeSpace += nodeCount * BINARY_POWER(i + MIN_POWER);  
        }
    }
    printf("Total Free Space: %d Available Memory Test: %d\n", totalFreeSpace, getAvailableMemory());
}
#endif