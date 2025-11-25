#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<string.h>
#include<signal.h>

#define MY_FIFO "./44_7.fifo"

void handler(int signo){
    printf("handler trigger: %s (%d) Triggered. Ignore\n", strsignal(signo), signo);
    return;
}

void process1(){    // 偶數秒 Process 1 執行
    printf("------------ [0] ------------\n");
    printf("Process 1: start\n");
    printf("Process 1: 在 Process 2 (Read open) 前 Open FIFO 會失敗 (ENXIO). 參考 Table 44-1\n");
    errno = 0;
    int fifofd = open(MY_FIFO, O_WRONLY | O_NONBLOCK);
    if(fifofd == -1 && errno==ENXIO){
        printf("Process 1: open failed. errno == ENXIO\n");
    }else if(fifofd == -1){
        perror("Process 1: open fifofd");
        exit(-1);
    }
    errno = 0;

    sleep(2);
    printf("------------ [2] ------------\n");

    printf("Process 1: Opening FIFO.\n");
    fifofd = open(MY_FIFO, O_WRONLY | O_NONBLOCK);
    if(fifofd == -1){
        perror("Process 1: open fifofd");
        exit(-1);
    }
    printf("Process 1: Success.\n");

    sleep(2);
    printf("------------ [4] ------------\n");
    if(write(fifofd, "A",1) == -1){
        perror("Process 1 Write Failed");
        exit(-1);
    }
    printf("Process 1: Write 'A' into FIFO.\n");

    sleep(2);
    printf("------------ [6] ------------\n");
    printf("Process 1: 在 Process 2 (Read) 關閉的時候 write\n");
    signal(SIGPIPE, handler);
    if(write(fifofd, "C",1) == -1){
        perror("Process 1 Write Failed");
    }
    close(fifofd);

    printf("Process 1: Stop.\n");
}


void process2(){    // 奇數秒 Process 2 執行
    printf("------------ [1] ------------\n");
    printf("Process 2: start\n");
    printf("Process 2: Opening FIFO.\n");
    int fifofd = open(MY_FIFO, O_RDONLY | O_NONBLOCK);
    if(fifofd == -1){
        perror("Process 2: open fifofd");
        exit(-1);
    }
    printf("Process 2: Success.\n");

    sleep(2);
    printf("------------ [3] ------------\n");
    printf("Process 2: 在 FIFO 沒有資料的情況下 read\n");
    char recv;
    int status = read(fifofd, &recv, 1);
    if(status == -1){
        perror("Process 2: read failed");
    }

    sleep(2);
    printf("------------ [5] ------------\n");
    printf("Process 2: 在 FIFO 有資料的情況下 read\n");
    status = read(fifofd, &recv, 1);
    if(status == -1){
        perror("Process 2: read failed");
        exit(-1);
    }
    printf("Process 2: Read '%c' from FIFO\n", recv);

    // sleep(2);
    // printf("------------ [5] ------------\n");
    printf("Process 2: Close FIFO fd (read)\n");
    close(fifofd);

    printf("Process 2: Stop.\n");
}

int main(){
    if(mkfifo(MY_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST){
        perror("mkfifo");
        exit(-1);
    }

    switch (fork()){
    case -1:
        perror("fork");
        exit(-1);
    
    case 0:
        process1();
        _exit(0);

    default:
        sleep(1);       // 讓 fifo 的兩邊不會馬上打開
        process2();
    }

    wait(NULL);
    
    unlink(MY_FIFO);
}