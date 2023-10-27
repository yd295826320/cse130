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
#define re "([A-Z]{1,8}) /([a-zA-Z0-9.-]{1,63}) (HTTP/[0-9].[0-9])\r\n"
#define hd "([a-zA-Z0-9.-]{1,128}): ([ -~]{0,128})\r\n"


void readrequest(int sd){
    char readbuff[buffsize] = {0};
    // char contbuff[buffsize];
    // ssize_t br = 0;
    // while (br < buffsize) {
    //     // printf("%s\n***************\n", readbuff);
    //     ssize_t od = read(sd, readbuff + br, buffsize - br);
    //     // printf("%ld\n***************\n", od);
    //     if (od <= 0) // reached end of input
    //         break;
    //     // if (od == -1) {
    //     //     status(500, sd, 0 ,0);
    //     //     return;
    //     // }
    //     br += od;
    // }
    
    // int bytesread = recv(sd, readbuff, buffsize, 0);
    int bytesread = read(sd, readbuff, buffsize);
    if (bytesread < 0){
        status(500, sd, 0 ,0);
        return;
    }
    // read(sd, readbuff + bytesread, buffsize - bytesread);
    // read(sd, readbuff, buffsize);

    // strcpy(contbuff,readbuff);
    // char *cont = strstr(contbuff, "\r\n\r\n");
    // if (cont == NULL){
    //     status(400, sd, 0, 0);
    //     return;
    // }

    // printf("%s\n***************\n", readbuff);
    // char Method[8];
    // char URI[64];
    // char Version[10];
    char *Method = NULL;
    char *URI = NULL;
    char *Version = NULL;
    char *eol = NULL;

    regex_t regex;
    regmatch_t matches[4];
    int rc = regcomp(&regex, re, REG_EXTENDED);
    // if (rc != 0) {
    //     status(500, sd, 0 ,0);
    //     return;
    // }
    rc = regexec(&regex, readbuff, 4, matches, 0);
    if (rc == 0) {
        Method = readbuff;
        URI = readbuff + matches[2].rm_so;
        Version = readbuff + matches[3].rm_so;
        eol = readbuff + matches[3].rm_eo + 2;

        
        // send(sd, Method, strlen(Method), 0);

        Method[matches[1].rm_eo] = '\0';
        URI[matches[2].rm_eo - matches[2].rm_so] = '\0';
        Version[matches[3].rm_eo - matches[3].rm_so] = '\0';

        // char t[buffsize];
        // sprintf(t, "Method %s\nURI %s\nVersion %s\neol %s\n", Method, URI, Version, eol);
        // send(sd, t, strlen(t), 0);
        
        
        
    }
    else {
        Method = NULL;
        URI = NULL;
        Version = NULL;
        eol = NULL;

        status(400, sd, 0, 0);
        return;
    }

    // printf("Method %s\nURI %s\nVersion %s\neol %s\n************\n", Method, URI, Version, eol);

    // for (unsigned long int j = 0; j < strlen(eol); j++){
    //     if (eol[j] == 13){
    //         printf("\\r\n");
    //     }
    //     else if (eol[j] == 10){
    //         printf("\\n\n");
    //     }
    //     else{
    //         printf("%c\n", eol[j]);
    //     }
    // }
    
    

    //sscanf(readbuff, "%s %s %s", Method, URI, Version);

    // if (strlen(Method) == 0 || strlen(URI) == 0 || strlen(Version) == 0){
    //     status(400, sd, 0, 0);
    //     return;
    // }

    if (strcmp(Version, "HTTP/1.1") != 0){
        status(505, sd, 0, 0);
        return;
    }


     // send(sd, eol, strlen(eol), 0);
    char putbuff[buffsize] = {0};
    strcpy(putbuff, eol);
    // char *temp = NULL;
    // char *templen = NULL;
    int contlen = 0;
    regmatch_t m2[3];
    rc = regcomp(&regex, hd, REG_EXTENDED);
    int index = 0;
    int check = 0;
    while ((rc = regexec(&regex, putbuff + index, 3, m2, 0)) == 0) {
        
        char key[128];   
        char value[128]; 
        memset(key, 0, sizeof(key));
        memset(value, 0, sizeof(value));
        // Copy the key and value from the matched substring
        strncpy(key, putbuff + index + m2[1].rm_so, m2[1].rm_eo - m2[1].rm_so);
        strncpy(value, putbuff + index + m2[2].rm_so, m2[2].rm_eo - m2[2].rm_so);

        // printf("key:%s\nvalue:%s\nindex:%i\n***********\n",key, value, index);
        if (strcmp(key, "Content-Length") == 0) {
            sscanf(value, "%d", &contlen);
            // printf("%d\n", contlen);
            check += 1;
        }
        eol = putbuff + m2[2].rm_eo + index + 2;
        // Update the current index for the next match
        index += m2[0].rm_eo;
        // printf("%d\n********\n",index);
        
    }

    // char t[buffsize];
    // sprintf(t, "temp %s\ncontlen %s\n\neol %s\n", temp, contlen, eol);
    // send(sd,t,strlen(t),0);

    // printf("eol %s\n************\n", eol);

    if (eol[0] != 13 || eol[1] != 10){
        status(400, sd, 0, 0);
        return;
    }

    // char *eol = strstr(readbuff, "\r\n\r\n");
    // if (eol == NULL){
    //     status(400, sd, 0, 3);
    //     return;
    // }

    // if (URI[0] != 47){
    //     status(400, sd, 0, 4);
    //     return;
    // }
    // else{
    //     for (unsigned long int i = 1; i < strlen(URI); i++){
    //         char c = URI[i];
    //         if (isalpha(c) == 0 && isdigit(c) == 0 && c != 45 && c != 46){
    //             status(400, sd, 0, 0);
    //             return; 
    //         }
    //     }
    // }

    // char *newURI = URI + 1;

    regfree(&regex);

    if (strcmp(Method, "GET") == 0){
        // if (strcmp(eol,"\r\n") != 0){
        //     // printf("here\n");
        //     status(400, sd, 0, 0);
        //     return;
        // }
        orderget(sd, URI);
    }

    else if(strcmp(Method, "PUT") == 0){
        // char *contentlen = strstr(readbuff, "Content-Length:");
        // if (contentlen == NULL){
        //     status(400, sd, 0, 0);
        //     return; 
        // }
        // char temp[buffsize];
        // int contlen;
        // sscanf(contentlen,"%s %d", temp, &contlen);
        if (check < 1){
            status(400, sd, 0, 0);
            return;
        }
        
        orderput(sd, URI, contlen, eol + 2);
    }
    else{
        status(501, sd, 0, 0);
        return; 
    }

    return;


}

void status(int code, int sd, int needok, long int contlen){
    char message[buffsize] = {0};
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
        // sprintf(message,  "%ld\n", contlen);
        // send(sd, message, strlen(message), 0);
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
























