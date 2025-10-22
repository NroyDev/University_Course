#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(){
    // get max path
    errno = 0;
    long pathmaxlen = pathconf(".", _PC_PATH_MAX);
    if(pathmaxlen == -1 && errno!=0){
        perror("Error - pathconf");
        exit(errno);
    }else if(pathmaxlen == -1){  // limit is indeterminate
        fprintf(stderr, "Warning - pathconf _PC_PATH_MAX limit is indeterminate\n");
        pathmaxlen = 4096;
    }
    const long PATH_MAX_LEN = pathmaxlen;

    char *buf = (char*)malloc(sizeof(char)*(PATH_MAX_LEN+1));
    if(buf == NULL){
        perror("Error - malloc");
        exit(errno);
    }
    if(getcwd(buf, PATH_MAX_LEN+1) == NULL){
        perror("Error - getcwd");
        exit(errno);
    }
    fprintf(stdout, "%s\n", buf);

    free(buf);
    return 0;
}