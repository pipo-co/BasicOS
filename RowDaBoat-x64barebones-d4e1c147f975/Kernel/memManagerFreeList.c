// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef BUDDY
#include <memoryManager.h>
#include <lib.h>
#include <screenDriver.h>

#define HEAP_SIZE (128 * 1024 * 1024) //128 MiB 
#define BLOCK_SIZE sizeof(node)
#define HEAP_TOTAL_BLOCKS (HEAP_SIZE / BLOCK_SIZE)
#define HEAP_BASE (0x600000)  //Posicion de memoria 6MiB, 1 MiB despues despues del comienzo de la seccion de datos de Userland.

typedef uint64_t Align;
union node{
    struct{
        union node *ptr;
        uint32_t size; 
    }s;
    Align x;
};
typedef union node node;

static void joinMem(node *left, node *right);

static node* first;

void initMM(){
    first = (node*)HEAP_BASE;
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

    if(bp < (node*)HEAP_BASE || bp >= (node*)(HEAP_BASE + HEAP_SIZE)){
        println("Free: Pointer Out of Bounds");
        return 1;
    }

    if(((uintptr_t)bp - (uintptr_t)HEAP_BASE) % BLOCK_SIZE != 0){
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
    
    uint32_t freeBlocks = 0;

    for(node *ptr = first; ptr != NULL; freeBlocks += ptr->s.size, ptr = ptr->s.ptr);
    
    return freeBlocks * BLOCK_SIZE;
}

void printList(){
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