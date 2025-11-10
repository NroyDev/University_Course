#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#define BUF_SIZE 4096

void parent(int pipefd1[], int pipefd2[]){
    fprintf(stdout, "Parent Start working...\n");
    // pipefd1: parent => child
    // pipefd2: child  => parent
    FILE* fwrite = fdopen(pipefd1[1], "w");
    FILE* fread  = fdopen(pipefd2[0], "r");
    if(fwrite == NULL || fread == NULL){
        perror("Error - fdopen()");
        exit(errno);
    }
    if(close(pipefd1[0]) == -1 || close(pipefd2[1])==-1){
        perror("Error - close()");
        exit(errno);
    }

    // ----------------- task -----------------
    char buf[BUF_SIZE];
    errno = 0;
    while(fgets(buf, BUF_SIZE, stdin) != NULL){
        int size = strlen(buf);
        fprintf(fwrite, "%s", buf);
        fflush(fwrite);
        fgets(buf, size+1, fread);
        fprintf(stdout, "%s", buf);
        if(errno != 0){
            perror("Error - in fgets loop");
            exit(errno);
        }
    }
    if(errno != 0){
        perror("Error - fegts()");
        exit(errno);
    }

    // ----------------- close fd fp -----------------
    if(fclose(fwrite) != 0 || fclose(fread) != 0){
        perror("Error - fclose()");
        exit(errno);
    }
    if(close(pipefd1[1]) == -1 || close(pipefd2[0])==-1){
        perror("Error - close()");
        exit(errno);
    }

    return;
}

void child(int pipefd1[], int pipefd2[]){
    fprintf(stdout, "Child Start working...\n");
    // pipefd1: parent => child
    // pipefd2: child  => parent
    FILE* fwrite = fdopen(pipefd2[1], "w");
    FILE* fread  = fdopen(pipefd1[0], "r");
    if(fwrite == NULL || fread == NULL){
        perror("Error - fdopen()");
        exit(errno);
    }
    if(close(pipefd1[1]) == -1 || close(pipefd2[0])==-1){
        perror("Error - close()");
        exit(errno);
    }

    // ----------------- task -----------------
    char buf[BUF_SIZE];
    errno = 0;
    while(fgets(buf, BUF_SIZE, fread) != NULL){
        int size = strlen(buf);
        for(int i=0; i<size; ++i){
            if('a' <= buf[i] && buf[i] <= 'z'){
                buf[i] ^= 32;
            }
        }
        fprintf(fwrite, "%s", buf);
        fflush(fwrite);
        if(errno != 0){
            perror("Error - in fgets loop");
            exit(errno);
        }
    }
    if(errno != 0){
        perror("Error - fegts()");
        exit(errno);
    }

    // ----------------- close fd fp -----------------
    if(fclose(fwrite) != 0 || fclose(fread) != 0){
        perror("Error - fclose()");
        exit(errno);
    }
    if(close(pipefd1[0]) == -1 || close(pipefd2[1])==-1){
        perror("Error - close()");
        exit(errno);
    }
    return;
}

int main(){
    // pipefd1: parent => child
    // pipefd2: child  => parent
    int pipefd1[2], pipefd2[2];
    if(pipe(pipefd1) == -1 || pipe(pipefd2) == -1){
        perror("Error - pipe()");
        exit(errno);
    }

    switch(fork()){
    case -1:
        perror("Error - fork()");
        exit(errno);
    
    case 0:
        // child
        child(pipefd1, pipefd2);
        break;
        
    default:
        // parent
        parent(pipefd1, pipefd2);
        break;
    }

    return 0;
}