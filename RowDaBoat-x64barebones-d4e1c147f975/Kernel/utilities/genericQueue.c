#include <stddef.h>
#include <genericQueue.h>
#include <memoryManager.h>

int enqueue(genericQueue * q, void * data){

    if(q == NULL || data == NULL)
        return -1;

    genericQueueNode * node = malloc2(sizeof(genericQueueNode));
    if(node == NULL)
        return -1;

    node->data = data; 

    if(q->first == NULL)
        q->first = node;
    else
        q->last->next = node;

    q->last = node;
    node->next = NULL;

    return 0;
}

void * dequeue(genericQueue * q){
    if(q == NULL || isQueueEmpty(q))
        return NULL;

    genericQueueNode * node = q->first;
    void * ans = node->data;

    if(q->last == node)
        q->last = NULL;

    q->first = q->first->next;

    free2(node);

    return ans;
}

int isQueueEmpty(genericQueue * q){
    return q == NULL || q->first == NULL;
}
