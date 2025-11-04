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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void kill_comsumers(int signo){
    for(int i=0;i<num_threads;++i){
        if(pthread_kill(threads[i], signo) != 0){
            printf("kill failed\n");
            fflush(stdout);
        }
    }
    // kill(getpid(), signo);
}

void wait_comsumers(){
    for(int i=0;i<num_threads;++i){
        pthread_join(threads[i], NULL);
    }
}

int prod_cnt = 0;
int VARIABLE = 0;
void producer_handler(int signo){
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
    pthread_mutex_lock( &mutex ); // 上鎖
    // fprintf(stdout, "Child Invoked\n");
    if(VARIABLE == '#'){
        if(DEBUG){
            printf("%d receive %c\n", getpid(), VARIABLE);
        }
        printf("Consumer %d: %d\n", getpid(), cnt/num_threads);
        pthread_mutex_unlock( &mutex ); // 上鎖
        pthread_exit(NULL);
    }else{
        if(DEBUG){
            printf("%d receive %d\n", getpid(), VARIABLE); 
        }
        cnt+= VARIABLE;
    }
    fflush(stdout);
    pthread_mutex_unlock( &mutex ); // 上鎖
}

void comsumer(void* args){
    // fprintf(stdout, "Using comsumer %lu\n", threads[*(int*)args]);
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