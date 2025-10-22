#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void usage(){
    fprintf(stdout, "Usage: mycatfilename\n");
}

int main(int argc, char** argv){
    if(argc != 2){
        usage();
        exit(-1);
    }

    FILE* fp = fopen(argv[1], "r");
    if(fp == NULL){
        perror("Error - fopen");
        exit(errno);
    }

    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE];
    errno = 0;
    while(fgets(buf, BUF_SIZE, fp) != NULL){
        fprintf(stdout, "%s", buf);
    }
    if(errno != 0){
        perror("Error - fgets");
        exit(errno);
    }


    if(fclose(fp) == EOF){
        perror("Error - fclose");
        exit(errno);
    }
}