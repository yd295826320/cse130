// Asgn 2: A simple HTTP server.
// By: Eugene Chou
//     Andrew Quinn
//     Brian Zhao

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"
#include "rwlock.h"
#include "ht.h"

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

typedef struct logobj{     
    char * method;
    char* uri;
    int status;
    int request_id;
}logobj;

typedef logobj* log_t;

typedef struct Threadobj
{
    pthread_t thread;
    int id;
    ht *rwlockHT;
    queue_t *q;
    int connfd;
    conn_t *conn;
} Threadobj;

typedef Threadobj* Thread;

void handle_connection(Thread);

void handle_get(Thread threads, log_t log);
void handle_put(Thread threads, log_t log);
void handle_unsupported(Thread threads, log_t log);

void log_writer(log_t log_info){                               
    //pthread_mutex_lock(&log_mutex);
    if (log_info -> method == NULL){
        return;
    }
    fprintf(stderr, "%s,%s,%d,%d\n", log_info -> method, log_info -> uri, log_info -> status, log_info -> request_id);                         
    //pthread_mutex_unlock(&log_mutex);
    return;
}

void worker(Thread threads){
    while(1){
        uintptr_t connfd = 0;
        queue_pop(threads -> q, (void **) &connfd);
        threads -> connfd = connfd;
        handle_connection(threads);
    }
}

int main(int argc, char **argv) {
    if (argc < 4) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //use getopt to get the number of threads defult is 4
    int opt = 0;
    uint16_t t = 4;                                    
    while((opt = getopt(argc, argv, "t:"))!=-1){
        switch(opt){
            case 't':                                      
                t = atoi(optarg);
                if (t <= 0){
                    warnx("invalid thread size");
                    exit(EXIT_FAILURE);
                }
                break;
            case '?':
                if (optopt == 't') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option -%c\n", optopt);
                }
                exit(EXIT_FAILURE);
            break;
        }
    }
    //fprintf(stdout, "here");
    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[3], &endptr, 10);
    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[3]);
        return EXIT_FAILURE;
    }

    //int t = argv[0];
    queue_t *q = queue_new(t);
    // pthread_t *threads = malloc(sizeof(pthread_t));
    Thread threads[t];
    ht* rwlockht = ht_create();

    for (int i = 0; i < t; i++){
        threads[i] = malloc(sizeof(Threadobj));
        threads[i] -> id = i;
        threads[i] -> rwlockHT = rwlockht;
        threads[i] -> q = q;
        pthread_create(&threads[i] -> thread, NULL, (void *(*) (void *))worker, (void*)threads);  
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    while (1) {
        uintptr_t connfd = listener_accept(&sock);
        queue_push(q, (void *)connfd);
        debug("%tu", connfd);
    }
    //free(threads);
    for (int i = 0; i < t; i++){
        ht_destroy(threads[i] -> rwlockHT);
        free(threads[i]);
    }
    return EXIT_SUCCESS;
}

void handle_connection(Thread threads) {

    log_t log = malloc(sizeof(logobj));

    threads -> conn = conn_new(threads -> connfd);

    const Response_t *res = conn_parse(threads -> conn);

    const Request_t *req = conn_get_request(threads -> conn);

    log -> method = (char *)(req);

    debug("%s: %d", log -> method, errno);

    if (res != NULL) {
        
        log -> uri = conn_get_uri(threads -> conn);
        log -> status = response_get_code(res);
        log -> request_id = atoi(conn_get_header(threads -> conn, "Request-Id"));
        log_writer(log);
        conn_send_response(threads -> conn, res);
    } else {
        debug("%s", conn_str(threads -> conn));
        
        if (req == &REQUEST_GET) {
            // lock
            handle_get(threads, log);
        } else if (req == &REQUEST_PUT) {
            handle_put(threads, log);
        } else {
            handle_unsupported(threads, log);
        }
    }
    free(log);
    conn_delete(&(threads -> conn));
}

void handle_get(Thread threads, log_t log) {

    char *uri = conn_get_uri(threads -> conn);
    debug("GET request not implemented. But, we want to get %s", uri);
    

    /*if (rwlock_ht[uri]){
        rwlock = rwlock_ht[uri];
    }
    else{
        rwlock = rwlock_new(N_WAY, 1);
        rwlock_ht[uri] = rwlock;
    }

    //lock the file for reading
    reader_lock(rwlock_ht[uri]);
    */

    rwlock_t* rw = ht_get(threads -> rwlockHT, uri);

    if (rw == NULL){
        rw = rwlock_new(N_WAY, 1);
        ht_set(threads -> rwlockHT, uri, rw);
    }

    reader_lock(rw);

    log -> uri = uri;

    const Response_t *res = NULL;
    int fd = open(uri, O_RDONLY, 0);
    if (fd < 0){
        debug("%s: %d", uri, errno);
        if (errno == EACCES) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        }
        else if(errno == ENOTDIR){
            res = &RESPONSE_NOT_FOUND;
            goto out;
        }
        else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }
    struct stat buffer;
    int status = fstat(fd, &buffer);
    if (status != 0){
        res = &RESPONSE_NOT_FOUND;
        goto out;
    }

    if (S_ISDIR(buffer.st_mode) == 0) {
        res = &RESPONSE_FORBIDDEN;
        goto out;
    }
    res = conn_send_file(threads -> conn, fd, buffer.st_size);

    if (res == NULL){
        res = &RESPONSE_OK;
    }
    reader_unlock(rw);
    close(fd);

out:
    log -> status = response_get_code(res);
    log -> request_id = atoi(conn_get_header(threads -> conn, "Request-Id"));
    log_writer(log);
    conn_send_response(threads -> conn, res);
}

void handle_unsupported(Thread threads, log_t log) {
    debug("handling unsupported request");
    log -> uri = conn_get_uri(threads -> conn);
    log -> status = 501;
    log -> request_id = atoi(conn_get_header(threads -> conn, "Request-Id"));
    log_writer(log);
    // send responses
    conn_send_response(threads -> conn, &RESPONSE_NOT_IMPLEMENTED);
}

void handle_put(Thread threads, log_t log) {

    char *uri = conn_get_uri(threads -> conn);
    const Response_t *res = NULL;
    debug("handling put request for %s", uri);

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    debug("%s existed? %d", uri, existed);

    //same as get
    rwlock_t* rw = ht_get(threads -> rwlockHT, uri);

    if (rw == NULL){
        rw = rwlock_new(N_WAY, 1);
        ht_set(threads -> rwlockHT, uri, rw);
    }

    writer_lock(rw);

    log -> uri = uri;

    // Open the file..
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) {
        debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }

    res = conn_recv_file(threads -> conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

    writer_unlock(rw);

    close(fd);

out:
    log -> status = response_get_code(res);
    log -> request_id = atoi(conn_get_header(threads -> conn, "Request-Id"));
    log_writer(log);
    conn_send_response(threads -> conn, res);
}



//questions: The worker thread and the dispatcher thread. : thread create n times and then push them 
// Do we only need to put a lock for read and write? What about the log? 
// Where do we need to send the message for log?



