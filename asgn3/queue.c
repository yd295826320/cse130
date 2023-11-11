#include "queue.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct queue{
    int head;
    int len;
    int tail;
    void **array;
    pthread_cond_t pushed;
    pthread_cond_t poped;
    pthread_mutex_t lock;

} queue_t;


queue_t *queue_new(int size){
    //allocate the memory
    queue_t *q = (queue_t *) malloc(sizeof(queue_t));
    pthread_cond_init(&q->poped, NULL);
    pthread_cond_init(&q->pushed, NULL);
    pthread_mutex_init(&q->lock, NULL);
    q -> len = size;
    q -> tail = 0;
    //set the array to all 0
    q -> array = (void **) calloc(size, sizeof(void *));

    return q;
}

void queue_delete(queue_t **q){
    
    queue_t* qq = *q;
    pthread_mutex_destroy(&qq->lock);
    pthread_cond_destroy(&qq->pushed);
    pthread_cond_destroy(&qq->poped);
    // free(*qq -> array);
    // (*q) -> array = NULL; 
    free(qq);
    *q = NULL;
    return;
}

bool queue_push(queue_t *q, void *elem){
    pthread_mutex_lock(&q -> lock);
    if (!q){
        return false;
    }
    //while the tail is more than the length of the queue we should wait until something is poped
    while(q -> tail >= q -> len){
        pthread_cond_wait(&q -> poped, &q -> lock);
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
    pthread_cond_signal(&q -> pushed);
    pthread_mutex_unlock(&q -> lock);

    return true;
}

bool queue_pop(queue_t *q, void **elem){
    pthread_mutex_lock(&q -> lock);
    if (!q || elem == NULL){
        return false;
    }
    //if the queue is empty then we should wait until something is pushed
    while (q -> tail == 0){
        pthread_cond_wait(&q -> pushed, &q -> lock);
    }
    
    *elem = q -> array[q -> head];
    void *pointer;
    //using a for loop to loop through and push everything up one
    for (int i = 1; i < q -> tail; i++){
        pointer = q -> array[i];
        q -> array[i - 1] = pointer;
    }
    q -> tail--;
    pthread_cond_signal(&q -> poped);
    pthread_mutex_unlock(&q -> lock);

    return true;
}

















