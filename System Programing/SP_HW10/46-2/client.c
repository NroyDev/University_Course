#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/msg.h>

#define MAX_MTEXT (2*sizeof(int))
struct mbuf{
    long mtype;
    char mtext[MAX_MTEXT];
};

// convert C_string to int
int stoi(const char* str){
    char *endptr;
    long val;
    errno = 0;
    val = strtol(str, &endptr, 10);
    if(errno != 0){
        perror("Error - strtol");
        exit(errno);
    }else if(endptr == str){
        fprintf(stderr, "No digits were found\n");
        exit(-1);
    }else if(*endptr != '\0'){
        fprintf(stdout,"Warning - Further characters after number: \"%s\" SKIPPED\n", endptr);
        return -1;
    }
    return val;
}

int main(int argc, const char** argv){
    if(argc > 2 || (argc == 2 && strcmp(argv[1], "--help")==0)){
        printf("%s [seq-len]\n", argv[0]);
        exit(-1);
    }
    int seqLen = 1;
    if(argc == 2){
        seqLen = stoi(argv[1]);
    }


    // 先開 message queue
    int msqid = msgget(ftok(".", 'Q'), 0666);
    if(msqid == -1){
        perror("msgget");
        exit(errno);
    }
    // printf("Client: msqid = %d\n", msqid);

    struct mbuf msg;
    msg.mtype = 1;      // to server
    *((int*)msg.mtext) = getpid();
    *((int*)msg.mtext+1) = seqLen;
    if(msgsnd(msqid, &msg, MAX_MTEXT, 0) == -1){
        perror("msgsnd");
        exit(errno);
    }

    int msgLen = msgrcv(msqid, &msg, MAX_MTEXT, 0, 0);  // server always listening on 0
    if(msgLen == -1){
        perror("msgrcv");
        exit(errno);
    }else if(msgLen < (int)MAX_MTEXT){
        printf("Client: msgLen is too small (%d).\n", msgLen);
        exit(errno);
    }
    printf("%d\n", *((int*)msg.mtext));

    return 0;
}