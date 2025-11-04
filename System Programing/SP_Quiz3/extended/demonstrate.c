#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#define N_THREADS 5

void handler(int signo) {
    printf("Thread %ld receive a signal %d\n", pthread_self(), signo);
}

void* function(void* arg) {
    printf("Thread %ld Created (pid=%d)\n", pthread_self(), getpid());
    while(1){
        pause();
    }
    return NULL;
}

int main() {
    // set handler
    pthread_t threads[N_THREADS];
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    sigaction(SIGUSR1, &sa, NULL);

    // Create Threads
    printf("Main Thread ID: %ld (pid=%d)\n", pthread_self(), getpid());
    for (int i = 0; i < N_THREADS; i++) {
        pthread_create(&threads[i], NULL, function, NULL);
    }

    sleep(1);
    printf("\n------- Using kill() -------\n");
    // Send the signal to the whole process, one of thread in process will receive
    printf("Now killing %d\n", getpid());
    kill(getpid(), SIGUSR1);
    printf("Kill 接受的是pid作為參數，會從該pid的process拿出一個沒有block這個訊號的thread去signal\n");

    sleep(1);
    printf("\n------- Using pthread_kill() -------\n");
    // Can Send signal to a sepcific thread
    printf("Now killing %ld\n", threads[1]);
    pthread_kill(threads[1], SIGUSR1);  
    printf("Kill 接受的是 pthread_t 作為參數，與 kill() 不同，可以指定特定的 thread 去 signal\n");

    sleep(1);
    printf("\nEND\n");
    return 0;
}
