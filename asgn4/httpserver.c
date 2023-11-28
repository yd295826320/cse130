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

typedef struct logobj {
    char *method;
    char *uri;
    int status;
    int request_id;
} logobj;

typedef logobj *log_t;

// typedef struct Threadobj
// {
//     pthread_t thread;
//     int id;
//     int connfd;
//     conn_t *conn;
// } Threadobj;

// typedef Threadobj* Thread;

void handle_connection(uintptr_t connfd);

void handle_get(conn_t *conn, log_t log);
void handle_put(conn_t *conn, log_t log);
void handle_unsupported(conn_t *conn);

queue_t *connet_q;

ht *rwlockht;

pthread_mutex_t lock;

void log_writer(log_t log_info) {
    if (log_info->method == NULL) {
        return;
    }
    fprintf(stderr, "%s,%s,%d,%d\n", log_info->method, log_info->uri, log_info->status,
        log_info->request_id);
    return;
}

void worker() {
    while (1) {
        uintptr_t connfd = 0;
        queue_pop(connet_q, (void **) &connfd);
        handle_connection(connfd);
        close(connfd);
    }
}

int main(int argc, char **argv) {
    if ((argc < 4) && (argc != 2)) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //use getopt to get the number of threads defult is 4
    int opt = 0;
    uint16_t t = 4;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            t = atoi(optarg);
            if (t <= 0) {
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
    // fprintf(stderr, "here");
    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[3], &endptr, 10);
    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[3]);
        return EXIT_FAILURE;
    }

    //int t = argv[0];
    connet_q = queue_new(t);
    pthread_t thread;

    rwlockht = ht_create();

    for (int i = 0; i < t; i++) {
        pthread_create(&thread, NULL, (void *(*) (void *) ) worker, NULL);
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    while (1) {
        uintptr_t connfd = listener_accept(&sock);
        queue_push(connet_q, (void *) connfd);
        //debug("%tu", connfd);
    }
    ht_destroy(rwlockht);
    queue_delete(&connet_q);
    return EXIT_SUCCESS;
}

void handle_connection(uintptr_t connfd) {

    log_t log = malloc(sizeof(logobj));

    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    const Request_t *req = conn_get_request(conn);

    //debug("%s: %d", log -> method, errno);

    if (res != NULL) {
        if (req == &REQUEST_GET) {
            log->method = "GET";
        } else if (req == &REQUEST_PUT) {
            log->method = "PUT";
        }
        char *temp = conn_get_header(conn, "Request-Id");
        log->uri = conn_get_uri(conn);
        log->status = response_get_code(res);
        if (temp) {
            log->request_id = atoi(temp);
        } else {
            log->request_id = 0;
        }
        log_writer(log);
        conn_send_response(conn, res);
    } else {
        //debug("%s", conn_str(threads -> conn));

        if (req == &REQUEST_GET) {
            log->method = "GET";
            handle_get(conn, log);
        } else if (req == &REQUEST_PUT) {
            log->method = "PUT";
            handle_put(conn, log);
        } else {
            handle_unsupported(conn);
        }
    }
    free(log);
    conn_delete(&conn);
}

void handle_get(conn_t *conn, log_t log) {

    char *uri = conn_get_uri(conn);
    //debug("GET request not implemented. But, we want to get %s", uri);

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
    pthread_mutex_lock(&lock);
    rwlock_t *rw = ht_get(rwlockht, uri);

    if (rw == NULL) {
        rw = rwlock_new(N_WAY, 1);
        ht_set(rwlockht, uri, rw);
    }
    pthread_mutex_unlock(&lock);
    reader_lock(rw);
    log->uri = uri;

    const Response_t *res = NULL;
    int fd = open(uri, O_RDONLY, 0);
    if (fd < 0) {
        //debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        } else if (errno == ENOTDIR || errno == ENOENT) {
            res = &RESPONSE_NOT_FOUND;
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }
    struct stat buffer;
    int status = fstat(fd, &buffer);
    if (status != 0) {
        res = &RESPONSE_NOT_FOUND;
        goto out;
    }

    if (S_ISDIR(buffer.st_mode) != 0) {
        res = &RESPONSE_FORBIDDEN;
        goto out;
    }
    res = conn_send_file(conn, fd, buffer.st_size);

    if (res == NULL) {
        res = &RESPONSE_OK;
    }

out:
    log->status = response_get_code(res);
    char *temp_header = conn_get_header(conn, "Request-Id");
    if (temp_header) {
        log->request_id = atoi(temp_header);
    } else {
        log->request_id = 0;
    }
    log_writer(log);
    if (res != &RESPONSE_OK) {
        conn_send_response(conn, res);
    }
    reader_unlock(rw);
    close(fd);
}

void handle_unsupported(conn_t *conn) {
    //we don't print log for unsupported
    //debug("handling unsupported request");
    // log -> uri = conn_get_uri(threads -> conn);
    // log -> status = 501;
    // char *temp_header = conn_get_header(threads -> conn, "Request-Id");
    // if (temp_header){
    //     log -> request_id = atoi(temp_header);
    // }
    // else{
    //     log -> request_id = 0;
    // }
    // log_writer(log);
    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}

void handle_put(conn_t *conn, log_t log) {

    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    //debug("%s existed? %d", uri, existed);

    //same as get
    pthread_mutex_lock(&lock);
    rwlock_t *rw;
    rw = ht_get(rwlockht, uri);

    if (rw == NULL) {
        rw = rwlock_new(N_WAY, 1);
        ht_set(rwlockht, uri, rw);
    }

    pthread_mutex_unlock(&lock);

    writer_lock(rw);

    log->uri = uri;

    // Open the file..
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd < 0) {
        //debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            goto out;
        }
    }

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }

out:
    log->status = response_get_code(res);

    char *temp_header = conn_get_header(conn, "Request-Id");

    if (temp_header) {
        log->request_id = atoi(temp_header);
    } else {
        log->request_id = 0;
    }
    log_writer(log);
    conn_send_response(conn, res);
    writer_unlock(rw);

    close(fd);
}
