#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define BUF_SIZE 256

int main(){
    char buf[BUF_SIZE];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    if((tmp = localtime(&t)) == NULL){
        perror("localtime");
        exit(errno);
    }
    if(strftime(buf, BUF_SIZE, "%b %-d(%a), %Y %-I:%M %p", tmp) == 0){
        fprintf(stderr, "strftime failed");
        exit(-1);
    }

    printf("%s\n", buf);
    return 0;
}