#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "asgn2_helper_funcs.h"
#include "request.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "need more!!!");
        exit(1);
    }
    Listener_Socket sk;
    int port;
    sscanf(argv[1], "%d", &port);

    if (port < 1 || port > 65535) {
        fprintf(stderr, "wrong port number");
        exit(1);
    }

    if (listener_init(&sk, port) == -1) {
        fprintf(stderr, "socket initialize failed");
        exit(1);
    }

    while (1) {
        int sd = listener_accept(&sk);
        if (sd < 0) {
            fprintf(stderr, "socket accept failed");
            exit(1);
        } else {
            readrequest(sd);
            //continue;
        }
        close(sd);
    }

    return 0;
}
