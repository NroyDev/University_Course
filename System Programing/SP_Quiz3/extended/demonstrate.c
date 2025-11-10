#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
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

    struct timeval start,end;
    printf("\n------- Using kill() -------\n");
    // Send the signal to the whole process, one of thread in process will receive
    printf("Now killing %d\n", getpid());
    gettimeofday(&start, NULL);
    kill(getpid(), SIGUSR1);
    gettimeofday(&end, NULL);
    printf("%ld us in toatl\n",(end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec));
    printf("Kill 接受的是pid作為參數，會從該pid的process拿出一個thread去signal (好像通常是 main thread)\n");
    // man 7 signal 是這樣說的
    //  A process-directed signal may be delivered to any one of the threads that does not currently have the signal blocked.  If more than one of the threads has the signal unblocked, then the ker‐
    //    nel chooses an arbitrary thread to which to deliver the signal.


    sleep(1);
    printf("\n------- Using pthread_kill() -------\n");
    // Can Send signal to a sepcific thread
    printf("Now killing %ld\n", threads[1]);
    gettimeofday(&start, NULL);
    pthread_kill(threads[1], SIGUSR1);  
    gettimeofday(&end, NULL);
    printf("%ld us in toatl\n",(end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec));
    printf("pthread_kill 接受的是 pthread_t 作為參數，與 kill() 不同，可以指定特定的 thread 去 signal\n");
    printf("然後 pthread_kill 好像比 kill() 更快\n");

    sleep(1);
    printf("\nEND\n");
    return 0;
}
