#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#define DEBUG 0

char* file_path;

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

double stod(const char* str){
    char *endptr;
    double val;
    errno = 0;
    val = strtod(str, &endptr);
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

void argvParser(char** argv, int* n, double* dt){
    *n = stoi(argv[1]);
    *dt = stod(argv[2]);
    file_path = argv[3];
}

int prod_cnt = 0;
pid_t pgrp = 0;
void producer_handler(int signo){
    if(DEBUG){
        printf("%d handler signaled\n", getpid());
    }
    fflush(stdout);
    if(prod_cnt < 5){
        int rand_num = (rand()%9)+1;
        FILE* fp = fopen(file_path, "w");
        fprintf(fp, "%d", rand_num);
        fclose(fp);

        if(DEBUG){
            printf("Main send %d USR1 to %d\n", rand_num, pgrp);
            fflush(stdout);
        }
        killpg(pgrp, SIGUSR1);
        prod_cnt++;
    }else{
        FILE* fp = fopen(file_path, "w");
        fprintf(fp, "%c", '#');
        fclose(fp);

        if(DEBUG){
            printf("Main send # USR1 to %d\n", pgrp);
            fflush(stdout);
        }
        killpg(pgrp, SIGUSR1);
        exit(0);
    }
    
}

void producer(double dt){
    struct itimerval it;

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    sigaddset(&act.sa_mask, SIGINT);
    act.sa_flags = 0;
    act.sa_handler = producer_handler;
    sigaction(SIGALRM, &act, (struct sigaction *)NULL);
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 100000;
    it.it_interval.tv_sec = (int)dt;
    it.it_interval.tv_usec = (int)((dt-(int)dt)*1000000);
    if(setitimer(ITIMER_REAL, &it,(struct itimerval *)NULL) == -1){
        perror("setitimer");
        exit(1);
    }

    while(1){
        pause();
    }
}

int cnt = 0;
void comsumer_handler(int signo){
    // printf("%d handler signaled\n", getpid());
    int val;
    char temp;
    FILE* fp = fopen(file_path, "r");
    fscanf(fp, "%c", &temp);
    if(temp == '#'){
        if(DEBUG){
            printf("%d receive %c\n", getpid(), temp);
        }
        printf("Consumer %d: %d\n", getpid(), cnt);
        exit(0);
    }else{
        rewind(fp);
        fscanf(fp, "%d", &val);
        if(DEBUG){
            printf("%d receive %d\n", getpid(), val); 
        }
        cnt+= val;
    }
    fclose(fp);
    fflush(stdout);
}

void comsumer(){
    if(DEBUG){
        printf("%d child here\n", getpid());
    }
    fflush(stdout);
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_flags = 0;
    act.sa_handler = comsumer_handler;
    sigaction(SIGUSR1, &act, (struct sigaction *)NULL);

    while(1){
        pause();
    }
}

void gen_comsumer(int n){
    setpgid(0,0);
    if(DEBUG){
        printf("%d %d\n", getpid(), getpgrp());
    }
    for(int i=0; i<n-1; ++i){
        pid_t pid = fork();
        if(pid<0){
            perror("fork");
            exit(errno);
        }else if(pid == 0){
            comsumer();
            break;
        }else{
            setpgid(pid, getpid());
        }
    }
    comsumer();
}



int main(int argc, char** argv){
    if(argc!=4){
        fprintf(stderr, "Usage: pro_con num_consumer interval filename\n");
    }
    int n;
    double dt;
    argvParser(argv, &n, &dt);


    pid_t pid = fork();
    switch (pid) {
    case -1:
        perror("fork");
        exit(errno);
    case 0:
        gen_comsumer(n);
    default:
        pgrp = pid;
        producer(dt);
    }


    return 0;
}