#include <stdio.h>
#include <stdlib.h>
#include "rwlock.h"
#include <pthread.h>




typedef struct rwlock{
    int active_reader;
    int waiting_reader;
    int waiting_writer;
    int priority;
    uint32_t nway;
    




} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n){





}
void rwlock_delete(rwlock_t **l);
void reader_lock(rwlock_t *rw);
void reader_unlock(rwlock_t *rw);
void writer_lock(rwlock_t *rw);
void writer_unlock(rwlock_t *rw);







