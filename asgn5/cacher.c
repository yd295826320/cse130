#include "ll.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <bits/getopt_core.h>

#define _GNU_SOURCE
typedef enum { FIFO, LRU, CLOCK } Policy;
typedef struct Cache {
    Policy p;
    Node *list;
    Node *history_list;
    int clock_pointer;
    int n;
    int CO;
    int CA;
} Cache;

typedef Cache *cache_t;

bool insert_fifo(Cache *cache, void *element);
bool insert_lru(Cache *cache, void *element);
bool insert_clock(Cache *cache, void *element);

int main(int argc, char **argv) {
    if (argc < 3) {
        warnx("wrong arguments: %s cache_num", argv[0]);
        return EXIT_FAILURE;
    }

    // ./cacher [-N size] <policy>
    int opt = 0;
    int n = 0;
    cache_t cache = malloc(sizeof(Cache));
    while ((opt = getopt(argc, argv, "N:FLC")) != -1) {
        switch (opt) {
        case 'N': n = atoi(optarg); break;
        case 'F': cache->p = FIFO; break;
        case 'L': cache->p = LRU; break;
        case 'C': cache->p = CLOCK; break;
        default: fprintf(stderr, "Usage: %s [-N size] [-F|-L|-C]\n", argv[0]); exit(EXIT_FAILURE);
        }
    }

    cache->list = NULL;
    cache->history_list = NULL;
    cache->clock_pointer = 0;
    cache->n = n;
    cache->CA = 0;
    cache->CO = 0;

    char l[100] = "";
    void *element = "";

    while (fgets(l, sizeof(l), stdin)) {
        size_t len = strlen(l);
        if (len > 0 && l[len - 1] == '\n') {
            l[len - 1] = '\0';
        }
        element = strdup(l);

        bool check = false;
        if (cache->p == FIFO) {

            check = insert_fifo(cache, element);
        } else if (cache->p == LRU) {
            check = insert_lru(cache, element);
        } else if (cache->p == CLOCK) {
            check = insert_clock(cache, element);
        }

        if (check == true) {
            fprintf(stdout, "HIT\n");
        } else {
            fprintf(stdout, "MISS\n");
        }
    }
    fprintf(stdout, "%d %d\n", cache->CO, cache->CA);
    deleteList(&(cache->list));
    deleteList(&(cache->history_list));
    free(element);
    free(cache);
    //free(l);
    return 0;
}

bool insert_fifo(cache_t cache, void *element) {
    if (cache == NULL) {
        warnx("invalid cache");
        exit(EXIT_FAILURE);
    }
    if (element == NULL) {
        warnx("invalid element");
        exit(EXIT_FAILURE);
    }
    if (get(cache->list, element)) {
        return true;
    }
    if (list_is_full(cache->list, cache->n)) {
        removeHead(&(cache->list));
        if (get(cache->history_list, element) == NULL) {
            cache->CO++;
            addEnd(&(cache->history_list), element);
        } else {
            cache->CA++;
        }
    } else {
        if (get(cache->history_list, element) == NULL) {
            cache->CO++;
            addEnd(&(cache->history_list), element);
        }
    }

    addEnd(&(cache->list), element);

    //printList(cache->list);
    return false;
}

bool insert_lru(cache_t cache, void *element) {
    if (cache == NULL) {
        warnx("invalid cache");
        exit(EXIT_FAILURE);
    }
    if (element == NULL) {
        warnx("invalid element");
        exit(EXIT_FAILURE);
    }
    if (get(cache->list, element)) {
        moveEnd(&cache->list, element);
        return true;
    }
    if (list_is_full(cache->list, cache->n)) {
        removeHead(&cache->list);
        if (!get(cache->history_list, element)) {
            cache->CO++;
            addEnd(&(cache->history_list), element);
        } else {
            cache->CA++;
        }
    } else {
        if (!get(cache->history_list, element)) {
            cache->CO++;
            addEnd(&(cache->history_list), element);
        }
    }

    addEnd(&(cache->list), element);
    //printList(cache->list);
    return false;
}

bool insert_clock(Cache *cache, void *element) {
    if (cache == NULL) {
        warnx("invalid cache");
        exit(EXIT_FAILURE);
    }
    if (element == NULL) {
        warnx("invalid element");
        exit(EXIT_FAILURE);
    }

    if (get(cache->list, element)) {
        Node *item = get(cache->list, element);
        item->ref = 1;
        //printList(cache->list);
        return true;
    }
    if (list_is_full(cache->list, cache->n)) {
        Node *item = get_index(cache->list, cache->clock_pointer);
        while (item->ref == 1) {
            item->ref = 0;
            cache->clock_pointer = (cache->clock_pointer + 1) % cache->n;
            item = get_index(cache->list, cache->clock_pointer);
        }

        overwrite(cache->list, cache->clock_pointer, element);
        cache->clock_pointer = (cache->clock_pointer + 1) % cache->n;
        //printList(cache->list);
        if (!get(cache->history_list, element)) {
            cache->CO++;
            addEnd(&(cache->history_list), element);
        } else {
            cache->CA++;
        }
        return false;
    } else {
        if (!get(cache->history_list, element)) {
            cache->CO++;
            addEnd(&(cache->history_list), element);
        }
    }

    addEnd(&(cache->list), element);
    //printList(cache->list);
    return false;
}
