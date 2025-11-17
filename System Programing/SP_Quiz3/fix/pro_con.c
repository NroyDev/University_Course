#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
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

void argvParser(char** argv, int* n, double* dt){
    *n = stoi(argv[1]);
    *dt = stod(argv[2]);
}

int num_threads = 0;
pthread_t* threads = NULL;
void kill_comsumers(int signo){
    for(int i=0;i<num_threads;++i){
        if(pthread_kill(threads[i], signo) != 0){
            printf("kill failed\n");
            fflush(stdout);
        }
    }
}

void wait_comsumers(){
    for(int i=0;i<num_threads;++i){
        pthread_join(threads[i], NULL);
    }
}

void producer_handler(int signo){return;}

int VARIABLE = 0;       // 透過全域變數 傳資料
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

    int prod_cnt = 0;   // fixed
    while(1){
        pause();

        if(DEBUG){
            printf("%d handler signaled\n", getpid());
        }
        fflush(stdout);
        if(prod_cnt < 5){
            int rand_num = (rand()%9)+1;
            VARIABLE = rand_num;
            kill_comsumers(SIGUSR1);
            prod_cnt++;
        }else{
            VARIABLE = '#';
            kill_comsumers(SIGUSR1);
            wait_comsumers();
            exit(0);
        }
    }
}

void comsumer_handler(int signo){return;}

void comsumer(void* args){
    // args is (int*)&id

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_flags = 0;
    act.sa_handler = comsumer_handler;
    sigaction(SIGUSR1, &act, (struct sigaction *)NULL);

    int cnt = 0;
    while(1){
        pause();

        if(VARIABLE == '#'){
            if(DEBUG){
                printf("%d receive %c\n", *(int*)args, VARIABLE);
            }
            printf("Consumer %d: %d\n", *(int*)args, cnt);
            free(args);
            pthread_exit(NULL);
        }else{
            if(DEBUG){
                printf("%d receive %d\n", *(int*)args, VARIABLE); 
            }
            cnt+= VARIABLE;
        }
    }
}

void gen_comsumer(){
    threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    for(int i=0;i<num_threads;++i){
        int* temp = (int*)malloc(sizeof(int));
        *temp = i;
        pthread_create(threads+i, NULL, (void*)comsumer, temp);
    }
}



int main(int argc, char** argv){
    if(argc!=3){
        fprintf(stderr, "Usage: pro_con num_consumer interval\n");
        exit(1);
    }
    double dt;
    argvParser(argv, &num_threads, &dt);
    gen_comsumer();
    producer(dt);

    return 0;
}