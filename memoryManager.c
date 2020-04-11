// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "memoryManager.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define HEAP_SIZE (1024 * 128) //128 MB 
#define BLOCK_SIZE sizeof(node)
#define HEAP_TOTAL_BLOCKS (HEAP_SIZE / BLOCK_SIZE)
#define HEAP_BASE 0x600000

typedef uint16_t Align;
union node{
    struct{
        union node *ptr;
        unsigned size; 
    }s;
    Align x;
};

typedef union node node;
static node* first;

static void joinMem(node *left, node *right);
static void printList();

static void testNacho1();
static void testNacho2();

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

void free2(void * ap){
    node* bp = (node*)ap - 1;
    node* p;
    bp->s.ptr = NULL;

    if(first == NULL){
        first = bp;
        return;
    }

    if(bp < first){   
        joinMem(bp, first);
        first = bp;
        return;
    }
    
    for(p = first; p->s.ptr != NULL && bp < p; p = p->s.ptr) //Unico a p antes de donde iria bp
         
    //Right Join
    joinMem(bp, p->s.ptr);
    printf("%u\n", bp->s.size);
    /* if(bp == NULL || p->s.ptr == NULL || bp >= p->s.ptr)
        return;

    if(bp + bp->s.size == p->s.ptr){
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    }else
        bp->s.ptr = p->s.ptr;
*/

    //Left Join
    joinMem(p, bp);
    /*
    if(p == NULL || bp == NULL || p >= bp)
        return;

    if(p + p->s.size == bp){
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    }else
        p->s.ptr = bp;
        */

}

static void joinMem(node *left, node *right){
    
    if(left == NULL || right == NULL || left >= right)
        return;

    if(left + left->s.size == right){
        left->s.size += right->s.size;
        left->s.ptr = right->s.ptr;
    }else
        left->s.ptr = right;
}

int main(int argc, char const *argv[]){
    char heapBase[HEAP_SIZE];
    initMM(heapBase);

    printList();
        
    int * arr = malloc2(32);
    void * arrBackUp = (void *) arr;

    for (int i = 0; i < 4; arr++, i++){
        *arr=i;
        //printf("%d\n", *arr);
    }
    
    free2(arrBackUp);
    printList();

    int * arr2 = malloc2(19);
    void * arrBackUp2 = (void *) arr2;
    for (int j = 0; j < 3; arr2++, j++){
        *arr2=j;
       // printf("%d\n", *arr2);
    }

    int * arr3 = malloc2(75000);
    void * arrBackUp3 = (void *) arr3;
    for (size_t i = 0; i < 10; arr3++, i++){
        *arr3=i;
        //printf("%d\n", *arr3);

    }

    free2(arrBackUp3);

    printList();
    
    int * arr4 = malloc2(75000);
    void * arrBackUp4 = (void *) arr4;
    for (size_t i = 0; i < 10; arr4++, i++){
        *arr4=i;
    }
    
    printList();
    
    free2(arrBackUp2);
    
    printList();

    free2(arrBackUp4);

    printList();
    
    // int * arr6 = malloc2(131055);
    // if(arr6 != NULL){
    //     void * arrBackUp6 = (void *) arr6;
    //     for (int j = 0; j < 3; arr6++, j++){
    //         *arr6=j;
    //     // printf("%d\n", *arr2);
    //     }
    //     printf("Holaaa\n");
    //     printList();

    //     free2(arrBackUp6);

    //     printList();
    // }

    // testNacho1();
    // testNacho2();

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
    printList();
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
    printList();
}