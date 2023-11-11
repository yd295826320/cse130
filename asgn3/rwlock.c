#include <stdio.h>
#include <stdlib.h>
#include "rwlock.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct rwlock {
    int readers;
    int writers;
    int waiting_writer;
    int waiting_reader;

    PRIORITY priority;
    int nway;
    int ncount;

    pthread_cond_t read;
    pthread_cond_t wrote;
    pthread_mutex_t lock;
    bool flag;

} rwlock_t;

rwlock_t *rwlock_new(PRIORITY p, uint32_t n) {
    rwlock_t *l = (rwlock_t *) malloc(sizeof(rwlock_t));
    pthread_cond_init(&l->read, NULL);
    pthread_cond_init(&l->wrote, NULL);
    pthread_mutex_init(&l->lock, NULL);
    l->readers = 0;
    l->writers = 0;
    l->waiting_writer = 0;
    l->waiting_reader = 0;
    l->priority = p;
    l->nway = n;
    l->ncount = 0;
    l->flag = false;

    return l;
}
void rwlock_delete(rwlock_t **l) {
    rwlock_t *rw = *l;
    pthread_mutex_destroy(&rw->lock);
    pthread_cond_destroy(&rw->read);
    pthread_cond_destroy(&rw->wrote);
    free(rw);
    *l = NULL;
    return;
}

int reader_wait(rwlock_t *rw) {
    // if the priority is readers we need to wait when there is active readers
    if (rw->priority == READERS) {
        return (rw->readers);
    }
    // if the priority is writers we need to wait when there is any kind of writer
    else if (rw->priority == WRITERS) {
        return (rw->waiting_writer || rw->writers);
    } else if (rw->priority == N_WAY) {
        // for nways if there is readers already then we need to see if there is any waiting writer.
        if (rw->readers) {
            if (rw->waiting_writer) {
                // if so we need to check if reach the n yet
                if (rw->ncount < rw->nway) {
                    return 0;
                } else {
                    // if so and the reader called unlock we need to let the waiting writer go
                    if (rw->flag == true) {
                        return (rw->waiting_writer);
                    } else {
                        return 0;
                    }
                }
            } else {
                return 0;
            }
        }
        // if there is no readers but writers we need to let the writers go first
        else if (rw->writers) {
            return (rw->writers);
        }
        // if no readers and writer, we need to check the flag to see if we reach the n or not
        else {
            if (rw->flag == true) {
                return (rw->waiting_writer);
            } else {
                return 0;
            }
        }
    } else {
        fprintf(stderr, "Error!");
        exit(1);
    }
}

int writer_wait(rwlock_t *rw) {
    // for all three of them, we need to wait when there is reader reading or writers writing, for atomic
    if (rw->priority == READERS) {
        return (rw->writers || rw->readers);
    } else if (rw->priority == WRITERS) {
        return (rw->writers || rw->readers);
    } else if (rw->priority == N_WAY) {
        if (rw->readers) {
            return (rw->readers);
        } else if (rw->writers) {
            return (rw->writers);
        }
        // if no readers or writers, we need to see if the readers reached the n yet, if not we need let the waiting reader go first.
        else {
            if (rw->flag == false) {
                return (rw->waiting_reader);
            } else {
                return 0;
            }
        }
    } else {
        fprintf(stderr, "Error!");
        exit(1);
    }
}

void rwlock_wake(rwlock_t *rw) {
    //wake up all the thread to check the condition again
    if (rw->waiting_reader) {
        pthread_cond_broadcast(&rw->wrote);
    }
    if (rw->waiting_writer) {
        pthread_cond_broadcast(&rw->read);
    }
}

//typedef enum {READERS, WRITERS, N_WAY} PRIORITY;
void reader_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->waiting_reader++;
    while (reader_wait(rw)) {
        pthread_cond_wait(&rw->wrote, &rw->lock);
    }
    rw->waiting_reader--;
    rw->readers++;

    if (rw->priority == N_WAY) {
        // if the flag has been set up and ncount is 0 we need to reset the flag
        if (rw->flag == true && rw->ncount == 0) {
            rw->flag = false;
        }

        if (rw->flag == false && rw->ncount < rw->nway) {
            rw->ncount++;
        }
    }
    pthread_mutex_unlock(&rw->lock);
}

void reader_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->readers--;
    if (rw->priority == N_WAY) {
        if (rw->ncount == rw->nway) {
            rw->flag = true;
        }
        if (rw->flag == true && rw->ncount > 0) {
            rw->ncount = 0;
        }
    }

    rwlock_wake(rw);

    pthread_mutex_unlock(&rw->lock);
}

void writer_lock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);
    rw->waiting_writer++;
    while (writer_wait(rw)) {
        pthread_cond_wait(&rw->read, &rw->lock);
    }

    rw->waiting_writer--;
    rw->writers++;
    pthread_mutex_unlock(&rw->lock);
}

void writer_unlock(rwlock_t *rw) {
    pthread_mutex_lock(&rw->lock);

    rw->writers--;

    if (rw->priority == N_WAY) {
        if (rw->flag == true) {
            rw->flag = false;
        }
    }

    rwlock_wake(rw);

    pthread_mutex_unlock(&rw->lock);
}
