#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "asgn2_helper_funcs.h"
#include "request.h"
#include "order.h"
#include <sys/stat.h>


void orderget(int sd, char *uri){
    struct stat statbuf;
    if (stat(uri, &statbuf) == 0){
        //check if it's a regular file
        if(S_ISREG(statbuf.st_mode) == 0){
            status(403, sd, 0, 0);
            return;
        }
        int fd = open(uri, O_RDONLY, 0);
        if (fd < 0){
            status(403, sd, 0, 0);
            return;
        }
        status(200, sd, 0, statbuf.st_size);
        pass_n_bytes(fd, sd, statbuf.st_size);
        close(fd);
        return;
    }
    else{
        status(404, sd, 0, 0);
        return;
    }


    return;


}

void orderput(int sd, char *uri, int contlen, char *cont){

    struct stat statbuff;
    if (stat(uri, &statbuff) == 0){
        int fd = open(uri, O_RDWR | O_CREAT | O_TRUNC, statbuff.st_mode);
        if (fd < 0){
            status(403, sd, 0, 0);
            return;
        }
        else{
            int byteswritten = write_n_bytes(fd, cont, strlen(cont));
            contlen -= byteswritten;
            pass_n_bytes(sd, fd, contlen);
            status(200, sd, 1, 3);
        }
        close(fd);
        return;
    }
    else{
        int fd = open(uri, O_RDWR | O_CREAT | O_TRUNC, 0777);
        if (fd < 0){
            status(403, sd, 0, 0);
            return;
        }
        else{
            int byteswritten = write_n_bytes(fd, cont, strlen(cont));
            contlen -= byteswritten;
            pass_n_bytes(sd, fd, contlen);
            status(201, sd, 0, 0);
        }
        close(fd);
        return;
    }



    return;
}























