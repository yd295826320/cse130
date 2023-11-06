#include "queue.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_cond_t pushed = PTHREAD_COND_INITIALIZER;
pthread_cond_t poped = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct queue{
    int head;
    int len;
    int tail;
    void **array;

} queue_t;


queue_t *queue_new(int size){
    //allocate the memory
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    q -> len = size;
    q -> tail = 0;
    //set the array to all 0
    q -> array = (void **) calloc(size, sizeof(void *));

    return q;
}

void queue_delete(queue_t **q){
    // free((*q) -> array);
    // (*q) -> array = NULL;
    free(*q);
    *q = NULL;
    return;
}

bool queue_push(queue_t *q, void *elem){
    pthread_mutex_lock(&lock);
    if (!q){
        return false;
    }
    //while the tail is more than the length of the queue we should wait until something is poped
    while(q -> tail >= q -> len){
        pthread_cond_wait(&poped, &lock);
    }
    //if it's empty we just add it to the head
    if (q -> tail == 0){
        q -> array[q -> head] = elem;
    }
    //if not then we add it to the tail
    else if(q -> tail > 0){
        q -> array[q -> tail] = elem;
    }
    q -> tail++;
    pthread_cond_signal(&pushed);
    pthread_mutex_unlock(&lock);

    return true;
}

bool queue_pop(queue_t *q, void **elem){
    pthread_mutex_lock(&lock);
    if (!q || elem == NULL){
        return false;
    }
    //if the queue is empty then we should wait until something is pushed
    while (q -> tail == 0){
        pthread_cond_wait(&pushed, &lock);
    }
    
    *elem = q -> array[q -> head];
    void *pointer;
    //using a for loop to loop through and push everything up one
    for (int i = 1; i < q -> tail; i++){
        pointer = q -> array[i];
        q -> array[i - 1] = pointer;
    }
    q -> tail--;
    pthread_cond_signal(&poped);
    pthread_mutex_unlock(&lock);

    return true;
}

















