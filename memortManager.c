#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE (0x100000 * 128) //128 MB 
#define BLOCK_SIZE sizeof(node)
#define HEAP_TOTAL_BLOCKS (HEAP_SIZE / BLOCK_SIZE)
#define HEAP_BASE 0x600000

typedef uint16_t Align;
union node{
    struct{
        union node *ptr;
        uint8_t size; 
    }s;
    Align x;
};

typedef union node node;
static node* first = HEAP_BASE;

void initMM(){
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

    for(p = first; p->s.ptr != NULL && bp < p->s.ptr; p = p->s.ptr); //Unico a p antes de donde iria bp

    if(p->s.ptr != NULL){
        //Right Join
        joinMem(bp, p->s.ptr);
    }
    //Left Join
    joinMem(p, bp);
}

void joinMem(node *left, node *right){
    
    if(left == NULL || right == NULL || left >= right)
        return;

    if(left + left->s.size == right){
        left->s.size += right->s.size;
        left->s.ptr = right->s.ptr;
    }else
        left->s.ptr = right;
}

int main(int argc, char const *argv[]){
    return 0;
}
