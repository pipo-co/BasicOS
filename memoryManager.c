// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

void initMM(void* init){
    first = init; //Deberia ser HEAP_BASE
    first->s.ptr = NULL;
    first->s.size = HEAP_TOTAL_BLOCKS;
}

//Heavily inspired in C malloc
void * malloc2(unsigned bytes){

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

    //A la segunda condicion le saque el igual. La tercera no la entiedno, no deberia ser menor?
    if( p == bp || bp < p + p->s.size/* || bp + bp->s.size >= p->s.ptr*/){
        printf("Free: Pointer Already Freed\n");
        if(bp < p + p->s.size)
        printf("toldya\n");
        return 3;
    }
    
    //Para mi esta esta demas
    // if(p->s.ptr == NULL && bp + bp->s.size > HEAP_SIZE + HEAP_BASE)
    //     return;

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
    
    initMM(heapBase);

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

size_t getAvailableMemory(){
    
    size_t freeBlocks = 0;

    //Primero guarda el size y despues itera. Hacia segmentation fault.
    for(node *ptr = first; ptr != NULL; freeBlocks += ptr->s.size, ptr = ptr->s.ptr);
    
    return freeBlocks * BLOCK_SIZE;
}