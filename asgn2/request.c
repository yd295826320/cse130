#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "asgn2_helper_funcs.h"
#include "request.h"
#include "order.h"
#include <sys/socket.h>
#include <regex.h>

#define buffsize 4096
// static const char *const re = "([A-Z]{1,8}) +(/[a-zA-Z0-9.-]{1,63}) +(HTTP/[0-9].[0-9])(\r\n)+";
// static const char *const hd = "([a-zA-Z0-9.-]{1,128}): ([ -~]{0,128})\r\n";


void readrequest(int sd){
    char readbuff[buffsize];

    int bytesread = recv(sd, readbuff, buffsize, 0);
    if (bytesread < 0){
        status(500, sd, 0 ,0);
        return;
    }
    
    char Method[8];
    char URI[64];
    char Version[10];

    // regex_t regex;
    // if (regcomp(&regex, re, REG_EXTENDED) != 0) {
    //     status(400, sd, 0, 0);
    //     return;
    // }

    sscanf(readbuff, "%s %s %s", Method, URI, Version);

    if (strlen(Method) == 0 || strlen(URI) == 0 || strlen(Version) == 0){
        status(400, sd, 0, 0);
        return;
    }

    if (strcmp(Version, "HTTP/1.1") != 0){
        status(400, sd, 0, 0);
        return;
    }

    char *eol = strstr(readbuff, "\r\n\r\n");
    if (eol == NULL){
        status(400, sd, 0, 0);
        return;
    }

    if (URI[0] != 47){
        status(400, sd, 0, 0);
        return;
    }
    else{
        for (unsigned long int i = 1; i < strlen(URI); i++){
            char c = URI[i];
            if (isalpha(c) == 0 && isdigit(c) == 0 && c != 45 && c != 46){
                status(400, sd, 0, 0);
                return; 
            }
        }
    }

    char *newURI = URI + 1;

    if (strcmp(Method, "GET") == 0){
        orderget(sd, newURI);
    }

    else if(strcmp(Method, "PUT") == 0){
        char *contentlen = strstr(readbuff, "Content-Length:");
        if (contentlen == NULL){
            status(400, sd, 0, 0);
            return; 
        }
        char temp[buffsize];
        int contlen;
        sscanf(contentlen,"%s %d", temp, &contlen);
        orderput(sd, newURI, contlen, eol + 4);
    }
    else{
        status(501, sd, 0, 0);
        return; 
    }

    return;


}

void status(int code, int sd, int needok, long int contlen){
    char message[buffsize];
    if (code == 200){
        sprintf(message, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", contlen);
        send(sd, message, strlen(message), 0);
        if (needok == 1){
            send(sd, "OK\n", 3, 0);
        }
    }
    else if(code == 201){
        send(sd, "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n", 51, 0);
    }
    else if(code == 400){
        send(sd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n", 60, 0);
    }
    else if(code == 403){
        send(sd, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n", 56, 0);
    }
    else if(code == 404){
        send(sd, "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n", 56, 0);
    }
    else if(code == 500){
        send(sd, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 23\r\n\r\nInternal Server Error\n", 80, 0);
    }
    else if(code == 501){
        send(sd, "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n", 68, 0);
    }
    else if(code == 505){
        send(sd, "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not Supported\n", 80, 0);
    }

    return;



}
























