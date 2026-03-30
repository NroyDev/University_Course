#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include "setting.h"

int main(){
    int sd;
    struct sockaddr_in server;
    struct hostent *host;
    /* create the socket for talking to server*/
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    /* get server internet address and put into addr
    * structure fill in the socket address structure
    * and connect to server */
    memset((char *) &server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    /* Server is local system. Get its name. */
    if((host = gethostbyname(SERVER_IP)) == NULL){
        perror("gethostbyname");
        exit(1);
    }
    memcpy((char *)&server.sin_addr, host->h_addr, host->h_length);
    /* connect to server */
    if(connect(sd, (struct sockaddr *)&server, sizeof(server))){
        perror("connect");
        exit(1);
    }

    
    char buf[BUF_SIZE];
    while(fgets(buf, BUF_SIZE, stdin) != NULL){
        if(buf[strlen(buf)-1]=='\n'){
            buf[strlen(buf)-1]='\0';
        }
        if(strcmp(buf, "exit") == 0){
            break;
        }
        
        // send to server
        if(write(sd, buf, strlen(buf)) == -1){
            perror("write");
            close(sd);
            exit(1);
        }
        // read result from server
        int len = read(sd, buf, BUF_SIZE-1);
        if(len <= 0){
            perror("read");
            len = 0;
            exit(1);
        }
        buf[len] = '\0';
        fprintf(stdout, "%s\n", buf);
        fflush(stdout);
    }
    close(sd);

    return 0;
}  
