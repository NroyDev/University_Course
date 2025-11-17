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

void argvParser(char** argv, int* n, double* dt, char** file_path){
    *n = stoi(argv[1]);
    *dt = stod(argv[2]);
    *file_path = argv[3];
}

void producer_handler(int signo){return;}      // ignore the sig

void producer(double dt, pid_t pgrp, const char* file_path){
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


    int prod_cnt = 0;           // fix
    while(1){
        pause();    
        // 把東西放下面就就好了 不會有那麼多分享給 handler 變數的問題

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
}

void comsumer_handler(int signo){return;}

void comsumer(int comsumer_id, const char* file_path){
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

    int cnt = 0;
    while(1){
        pause();
        // 把東西放下面就就好了 不會有那麼多分享給 handler 變數的問題
        // printf("%d handler signaled\n", getpid());
        int val;
        char temp;
        FILE* fp = fopen(file_path, "r");
        fscanf(fp, "%c", &temp);
        if(temp == '#'){
            if(DEBUG){
                printf("%d receive %c\n", comsumer_id, temp);
            }
            printf("Consumer %d: %d\n", comsumer_id, cnt);
            exit(0);
        }else{
            rewind(fp);
            fscanf(fp, "%d", &val);
            if(DEBUG){
                printf("%d receive %d\n", comsumer_id, val); 
            }
            cnt+= val;
        }
        fclose(fp);
        fflush(stdout);
    }
}

void gen_comsumer(int n, const char* file_path){
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
            comsumer(i, file_path);
            break;
        }else{
            setpgid(pid, getpid());
        }
    }
    comsumer(n-1, file_path);
}



int main(int argc, char** argv){
    if(argc!=4){
        fprintf(stderr, "Usage: pro_con num_consumer interval filename\n");
        exit(-1);
    }
    int n;
    double dt;
    char* filepath = NULL;
    argvParser(argv, &n, &dt, &filepath);


    pid_t pid = fork();
    switch (pid) {
    case -1:
        perror("fork");
        exit(errno);
    case 0:
        gen_comsumer(n, filepath);
    default:
        producer(dt, pid, filepath);
    }


    return 0;
}