#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(){
    int pipefd[2];
    pid_t pid;

    if(pipe(pipefd) == -1){
        perror("Error - pipe");
        exit(errno);
    }

    if(signal(SIGCHLD, SIG_IGN) == SIG_ERR){
        // 這段我從 man fork 下拿來的，上網查說這段是：
        // 忽略 child process signal，child Process 結束時，自動清理其狀態資源，避免其變成zombie
        perror("Error - signal");
        exit(errno);
    }
    pid = fork();
    switch (pid){
    case -1:
        perror("fork");
        exit(errno);
    case 0:
        close(pipefd[0]);
        close(STDOUT_FILENO);
        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("Error - dup2");
            exit(errno);
        }
        close(pipefd[1]);

        if(execlp("ls", "ls", "-l", NULL)==-1){
            // should not be executed below
            perror("Error - execlp");
            exit(errno);
        }

    default:
        close(pipefd[1]);
        FILE* fp_pipeR = fdopen(pipefd[0], "r");
        if(fp_pipeR == NULL){
            perror("Error - fdopen");
            exit(errno);
        }

        const int BUF_SIZE = 4096;
        char buf[BUF_SIZE];
        errno = 0;
        while(fgets(buf, BUF_SIZE, fp_pipeR) != NULL){
            fprintf(stdout, "%s", buf);
        }
        if(errno != 0){
            perror("Error - fgets");
            exit(errno);
        }

        fclose(fp_pipeR);
        close(pipefd[0]);
    }

    return 0;
}