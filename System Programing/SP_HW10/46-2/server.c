#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/msg.h>

#define MAX_MTEXT (2*sizeof(int))
struct mbuf{
    long mtype;
    char mtext[MAX_MTEXT];
};

int msqid = -1;
void remove_msgq(){
    if(msqid!=-1 && msgctl(msqid, IPC_RMID, NULL) == -1){
        perror("remove msgqueue: msgctl");
    }else{
        printf("\nServer: msg_queue remove Successfully.\n");
    }
}
void handler(int signo){
    exit(signo);
}

int main(){
    // 先開 message queue
    msqid = msgget(ftok(".", 'Q'), IPC_CREAT|IPC_EXCL|0666);
    if(msqid == -1){
        perror("msgget");
        exit(errno);
    }
    if(atexit(remove_msgq) != 0){
        perror("atexit");
        remove_msgq();
        exit(errno);
    }
    signal(SIGINT, handler);
    signal(SIGTSTP, handler);

    int seqNum = 0;     // 要傳的東西
    struct mbuf msg;
    printf("Server: start listening on %d\n", msqid);
    for(;;){
        int msgLen = msgrcv(msqid, &msg, MAX_MTEXT, 1, 0);  // server always listening on 0
        if(msgLen == -1){
            perror("msgrcv");
            exit(errno);
        }else if(msgLen < (int)MAX_MTEXT){
            printf("Server: msgLen is too small (%d). Discard.\n", msgLen);
            continue;
        }
        printf("Server: Receive request from Client(PID=%d), request seq-len=%d\n", *((int*)msg.mtext), *(((int*)msg.mtext)+1));

        int seqLen = *(((int*)msg.mtext)+1);
        msg.mtype = *((int*)msg.mtext);
        *((int*)msg.mtext) = seqNum;
        if(msgsnd(msqid, &msg, MAX_MTEXT, 0) == -1){
            perror("msgsnd");
            exit(errno);
        }
        seqNum += seqLen;
    }
}