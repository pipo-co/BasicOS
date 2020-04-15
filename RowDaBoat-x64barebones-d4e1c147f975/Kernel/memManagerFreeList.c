// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef BUDDY
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define BLOCK_SIZE sizeof(node)
#define HEAP_TOTAL_BLOCKS (heap_size / BLOCK_SIZE)

typedef uint64_t Align;
union node{
    struct{
        union node *ptr;
        uint32_t size; 
    }s;
    Align x;
};
typedef union node node;

static node *heap_base; //Asumimos 6 MiB
static uint32_t heap_size; //Asumimos 128 MiB
static uint32_t availableBlocks;

static void joinMem(node *left, node *right);

static node* first;

void initMM(void * heap_baseInit, uint32_t heap_sizeInit){
    heap_base = (node*)heap_baseInit;
    heap_size = heap_sizeInit;
    availableBlocks = HEAP_TOTAL_BLOCKS;

    first = heap_base;
    first->s.ptr = NULL;
    first->s.size = HEAP_TOTAL_BLOCKS;
}

//Heavily inspired in C malloc
void * malloc2(uint32_t bytes){
    if(bytes == 0)
        return NULL;

    node* p;
    node* prevp = first; 
    uint32_t blocks = (bytes % BLOCK_SIZE == 0) ? bytes/BLOCK_SIZE + 1 : bytes/BLOCK_SIZE + 2;

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
            availableBlocks -= p->s.size;
            return (void*)(p + 1);
        }
    }
    return NULL;
} 

int free2(void * ap){

    if(ap == NULL){
        println("Free: Pointer is NULL");
        return 1;
    }

    node* bp = (node*)ap - 1;

    if(bp < heap_base || bp >= heap_base + heap_size){
        println("Free: Pointer Out of Bounds");
        return 1;
    }

    if(((uintptr_t)bp - (uintptr_t)heap_base) % BLOCK_SIZE != 0){
        println("Free: Pointer is not a multiple of node");
        return 2;
    }
    
    node* p;

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
    
    //Ubico a p antes de donde iria bp
    for(p = first; p->s.ptr != NULL && p != bp && !(p < bp && bp < p->s.ptr); p = p->s.ptr);

    if(p == bp || bp < p + p->s.size){
        println("Free: Pointer Already Freed");
        return 3;
    }

    availableBlocks += bp->s.size; 
    //Right Join
    joinMem(bp, p->s.ptr);

    //Left Join
    joinMem(p, bp);

    return 0;
}

static void joinMem(node *left, node *right){
    
    if(left == NULL || (right != NULL && left >= right)){
        println("joinMem: Invalid Join");
        return;
    }

    if(left + left->s.size == right){
        left->s.size += right->s.size;
        left->s.ptr = right->s.ptr;
    }else
        left->s.ptr = right;
}

uint32_t getAvailableMemory(){
    
    return availableBlocks * BLOCK_SIZE;

    /* uint32_t freeBlocks = 0;

    for(node *ptr = first; ptr != NULL; freeBlocks += ptr->s.size, ptr = ptr->s.ptr);
    
    return freeBlocks * BLOCK_SIZE; */
}

void dumpMM(){
    uint64_t count = 0;
    node* iter = first;
    println("Free memory blocks list");
    if(iter == NULL)
        println("List is NULL");

    while(iter != NULL){
        printString("Node number: ");
        printint(count++);
        printString(". Start adress: "); 
        printint((uint64_t)iter);
        printString(". Available blocks: "); 
        printint((uint64_t)iter->s.size);
        printString(". Next free adress: "); 
        printint((uint64_t)iter->s.ptr);
        println(".");
        iter = iter->s.ptr;
    }
    putchar('\n');
}

#endif