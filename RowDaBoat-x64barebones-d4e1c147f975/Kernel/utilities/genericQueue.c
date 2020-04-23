#include <stdint.h>
#include <stddef.h>
#include <genericQueue.h>

int enqueue(genericQueue * q, genericQueueNode * node){
    if(q == NULL || node == NULL)
        return -1;

    if(q->first == NULL)
        q->first = node;
    else
        q->last->next = node;

    q->last = node;
    node->next = NULL;

    return 0;
}

genericQueueNode * dequeue(genericQueue * q){
    if(q == NULL || isQueueEmpty(q))
        return NULL;

    genericQueueNode * ans = q->first;

    if(q->last == ans)
        q->last = NULL;

    q->first = q->first->next;

    return ans;
}

int isQueueEmpty(genericQueue * q){
    return q == NULL || q->first == NULL;
}
