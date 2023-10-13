#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>  
                                     

#define BUFF_SIZE 4096
char buff[BUFF_SIZE];
ssize_t w;
char *order;
char *file;
char *temp_contlen;
int contlen;
char t;

int main(void) {
    ssize_t br = 0;
    while(br < BUFF_SIZE) {
        // fill buffer by reading from STDIN
        
        ssize_t od = read(STDIN_FILENO, buff + br, BUFF_SIZE - br);
        

        if (od == 0) // reached end of input
            break;
        if (od == -1){
            fprintf(stderr,"Operation Failed\n"); 
            exit(1);
        } 
        br += od;
    }       

        order = strtok(buff, "\n");

        if ((strcmp(order, "get") == 0) && (buff[br - 1] != '\n')){
            fprintf(stderr,"Invalid Command\n"); 
            exit(1);
        }
        
        file = strtok(NULL, "\n");
        

        
            


        //check if filename exist
        if (file){            
            //vaild file name                              
            if (strlen(file) > PATH_MAX){
                fprintf(stderr,"Invalid Command\n"); 
                exit(1);
            }


            if (strcmp(order, "get") == 0){
                temp_contlen = strtok(NULL, "");
                //extra stuff
                if (temp_contlen){
                    fprintf(stderr,"Invalid Command\n"); 
                    exit(1);
                }

                int fd = open(file, O_RDONLY, 0);
                
                if (fd < 0){
                    
                    fprintf(stderr,"Invalid Command\n"); 
                    exit(1);
                }
                while(1){
                    ssize_t bytesWritten = 0;
                    ssize_t rd = read(fd,buff,BUFF_SIZE);
                    if (rd == 0) // reached end of input
                        break;
                    if (rd == -1){
                        
                        fprintf(stderr,"Invalid Command\n"); 
                        exit(1);
                    }
                        
                    while (bytesWritten < rd) {
                        w = write(STDOUT_FILENO, buff + bytesWritten, rd - bytesWritten);
                        if (w == -1) {
                            
                            fprintf(stderr,"Operation Failed\n"); 
                            exit(1);
                        }
                        bytesWritten += w;
                    }
                }

                w = close(fd);
                if (w < 0){
                    fprintf(stderr,"Operation Failed\n"); 
                    exit(1);
                }

            }


            else if(strcmp(order, "set") == 0){
                temp_contlen = strtok(NULL, "\n");
                
                //if content or content length doesn't exist
                if (temp_contlen != NULL){
                    contlen = atoi(temp_contlen);
                    if ((strcmp(temp_contlen,"0") != 0) && contlen <= 0){
                        fprintf(stderr,"Invalid Command\n");  
                        exit(1);
                    }
                }
                else{
                    fprintf(stderr,"Invalid Command\n");  
                    exit(1);
                }

                int fd = open(file, O_CREAT|O_RDWR|O_TRUNC, 0777);
                
                if (fd < 0){
                    fprintf(stderr,"Invalid Command\n");  
                    exit(1);
                }


                ssize_t bytesread = 4 + strlen(file) + 1 + strlen(temp_contlen) + 1;
                char *cont;
                ssize_t cl;
                cont = buff + bytesread;
                if (contlen < (BUFF_SIZE - bytesread)){
                    cl = contlen;
                }
                else{
                    cl = BUFF_SIZE - bytesread;
                }
                


                // if there is no content
                if (cont == NULL){
                    w = close(fd);
                    if (w < 0){
                        fprintf(stderr,"Operation Failed\n"); 
                        exit(1);
                    }
                    ssize_t so = write(STDOUT_FILENO, "OK\n", 3);
                    if (so < 0){
                        
                        fprintf(stderr,"Operation Failed\n"); 
                        exit(1);
                    }
                    return 0;
                }
                

                ssize_t bw = 0;
                while(bw < cl){
                    w = write(fd, cont + bw, cl - bw);
                    if (w == 0){
                        break;
                    }
                    if (w == -1) {
                        fprintf(stderr,"Operation Failed\n"); 
                        exit(1);
                    }
                    bw += w;
                }
                
                contlen -= bw;
                ssize_t byteslefttowrite = 0;
                while(byteslefttowrite < contlen){
                    ssize_t bytesWritten = 0;
                    ssize_t rd = read(STDIN_FILENO,buff,BUFF_SIZE);
                    if (rd == 0) // reached end of input
                        break;
                    if (rd == -1){
                        
                        fprintf(stderr,"Invalid Command\n"); 
                        exit(1);
                    }
                        
                    while (bytesWritten < rd) {
                        w = write(fd, buff + bytesWritten, rd - bytesWritten);
                        if (w == -1) {
                            
                            fprintf(stderr,"Operation Failed\n"); 
                            exit(1);
                        }
                        bytesWritten += w;
                    }
                    byteslefttowrite += bytesWritten;
                }

                w = close(fd);
                if (w < 0){
                    
                    fprintf(stderr,"Operation Failed\n"); 
                    exit(1);
                }
                ssize_t so = write(STDOUT_FILENO, "OK\n", 3);
                if (so < 0){
                    
                    fprintf(stderr,"Operation Failed\n"); 
                    exit(1);
                }

            }

            
            else {
                fprintf(stderr,"Invalid Command\n"); 
                exit(1);
            }
        }
        else{
            fprintf(stderr,"Invalid Command\n"); 
            exit(1);
        }

        
       
    //}


    return 0;
}


// questions: for the content, what if there is huge amount of content and cannot fit into the buffer? 
// Can we consider the first read  can read out all the other context in the command?

