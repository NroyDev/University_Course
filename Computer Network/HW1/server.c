#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include "setting.h"

void task(int ns){
    // Read from client
    char buf[BUF_SIZE] = {};
    int len = 0;
    while((len = read(ns, buf, BUF_SIZE - 1)) > 0){
        buf[len] = '\0';
        fprintf(stdout, "PID %u received (%ld): %s\n", getpid(), strlen(buf), buf);
        fflush(stdout);
        if(strcmp(buf,"exit") == 0){
            break;
        }
        
        // compute
        int a, b;
        if(sscanf(buf, "%*c%d%*c%d%*c", &a, &b) != 2) {
            fprintf(stderr, "PID %u has Invalid input format: %s\n", getpid(), buf);
            strcpy(buf, "bad format");
        }else{
            sprintf(buf, "%d", a*b);
        }
        fprintf(stdout, "PID %u result: %s\n", getpid(), buf);
        fflush(stdout);
        

        // Send Result to client
        if(write(ns, buf, strlen(buf)) == -1){
            perror("write to socket");
        }
    }
    
    fprintf(stdout, "PID %u disconnected\n", getpid());
    fflush(stdout);
    close(ns);
}


int main(){
    struct sockaddr_in socketname, client;
    int sd, ns;
    socklen_t clientlen = sizeof(client);

    /* fill in socket address structure */
    memset((char *) &socketname, '\0', sizeof(socketname));
    socketname.sin_family = AF_INET;
    socketname.sin_port = htons(SERVER_PORT);
    socketname.sin_addr.s_addr = htonl(INADDR_ANY);     // listen all IP of this computer

    /* open socket */
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }
    /* bind socket to a name */
    if(bind(sd, (struct sockaddr *) & socketname, sizeof(socketname))){
        perror("bind");
        exit(1);
    }
    /* prepare to receive multiple connect requests */
    if(listen(sd, 128)){
        perror("listen");
        exit(1);
    }

    signal(SIGCHLD, SIG_IGN);
    while(1){
        clientlen = sizeof(client);
        if ((ns = accept(sd, (struct sockaddr *)&client, &clientlen)) == -1) {
            perror("accept");
            continue;
        }
        
        switch(fork()){
        case -1:
            perror("fork");
            close(ns);
            break;
        case 0:
            fprintf(stdout, "new connection arrived. Connection runs on process %u\n", getpid());
            fflush(stdout);
            task(ns);
            _exit(0);
            break;
        default:
            close(ns);
            break;
        }
    }
}