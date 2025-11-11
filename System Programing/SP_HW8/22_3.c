/* sig_speed_sigsuspend.c

   This program times how fast signals are sent and received.

   The program forks to create a parent and a child process that alternately
   send signals to each other (the child starts first). Each process catches
   the signal with a handler, and waits for signals to arrive using
   sigsuspend().

   Usage: $ time ./sig_speed_sigsuspend num-sigs

   The 'num-sigs' argument specifies how many times the parent and
   child send signals to each other.

   Child                                  Parent

   for (s = 0; s < numSigs; s++) {        for (s = 0; s < numSigs; s++) {
       send signal to parent                  wait for signal from child
       wait for a signal from parent          send a signal to child
   }                                      }
*/
#include <signal.h>
#include <sys/time.h>
#include "tlpi_hdr.h"
static void handler(int sig){}

#define TESTSIG SIGUSR1

void using_sigsuspend(int argc, char *argv[]){
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-sigs\n", argv[0]);

    int numSigs = getInt(argv[1], GN_GT_0, "num-sigs");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;
    if (sigaction(TESTSIG, &sa, NULL) == -1)
        errExit("sigaction");

    /* Block the signal before fork(), so that the child doesn't manage
       to send it to the parent before the parent is ready to catch it */

    sigset_t blockedMask, emptyMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, TESTSIG);
    if (sigprocmask(SIG_SETMASK, &blockedMask, NULL) == -1)
        errExit("sigprocmask");

    sigemptyset(&emptyMask);

    pid_t childPid = fork();
    switch (childPid) {
    case -1: errExit("fork");

    case 0:     /* child */
        for (int scnt = 0; scnt < numSigs; scnt++) {
            if (kill(getppid(), TESTSIG) == -1)
                errExit("kill");
            if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
                    errExit("sigsuspend");
            // write(STDOUT_FILENO, "CHILD: SIGUSR1 RECEIVED\n", strlen("CHILD: SIGUSR1 RECEIVED\n"));
        }
        _exit(EXIT_SUCCESS);

    default: /* parent */
        for (int scnt = 0; scnt < numSigs; scnt++) {
            if (sigsuspend(&emptyMask) == -1 && errno != EINTR)
                    errExit("sigsuspend");
            // write(STDOUT_FILENO, "PARENT: SIGUSR1 RECEIVED\n", strlen("PARENT: SIGUSR1 RECEIVED\n"));
            if (kill(childPid, TESTSIG) == -1)
                errExit("kill");
        }
        // exit(EXIT_SUCCESS);
        return;
    }
}


void using_sigwaitinfo(int argc, char *argv[]){
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-sigs\n", argv[0]);

    int numSigs = getInt(argv[1], GN_GT_0, "num-sigs");

    /* Block the signal before fork(), so that the child doesn't manage
       to send it to the parent before the parent is ready to catch it */

    sigset_t blockedMask, emptyMask;
    sigemptyset(&blockedMask);
    sigaddset(&blockedMask, TESTSIG);
    if (sigprocmask(SIG_SETMASK, &blockedMask, NULL) == -1)
        errExit("sigprocmask");

    sigemptyset(&emptyMask);

    pid_t childPid = fork();
    switch (childPid) {
    case -1: errExit("fork");

    case 0:     /* child */
        for (int scnt = 0; scnt < numSigs; scnt++) {
            if (kill(getppid(), TESTSIG) == -1)
                errExit("kill");
            if (sigwaitinfo(&blockedMask, NULL) == -1 && errno != EINTR)
                    errExit("sigprocmask");
            // write(STDOUT_FILENO, "CHILD: SIGUSR1 RECEIVED\n", strlen("CHILD: SIGUSR1 RECEIVED\n"));
        }
        _exit(EXIT_SUCCESS);

    default: /* parent */
        for (int scnt = 0; scnt < numSigs; scnt++) {
            if (sigwaitinfo(&blockedMask, NULL) == -1 && errno != EINTR)
                    errExit("sigprocmask");
            // write(STDOUT_FILENO, "PARENT: SIGUSR1 RECEIVED\n", strlen("PARENT: SIGUSR1 RECEIVED\n"));
            if (kill(childPid, TESTSIG) == -1)
                errExit("kill");
        }
        // exit(EXIT_SUCCESS);
        return;
    }
}

int main(int argc, char *argv[]){
    struct timeval start,end;
    printf("------- Using sigsuspend() -------\n");
    gettimeofday(&start, NULL);
    using_sigsuspend(argc, argv);
    gettimeofday(&end, NULL);
    long sigsuspend_time = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec);
    printf("%ld sec %ld us in toatl\n", sigsuspend_time/1000000, sigsuspend_time%1000000);
    printf("------- Using sigwaitinfo() -------\n");
    gettimeofday(&start, NULL);
    using_sigwaitinfo(argc, argv);
    gettimeofday(&end, NULL);
    long sigwaitinfo_time = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec);
    printf("%ld sec %ld us in toatl\n", sigwaitinfo_time/1000000,sigwaitinfo_time%1000000);
    printf("------- Difference -------\n");
    long diff = sigsuspend_time-sigwaitinfo_time;
    printf("speed difference = %ld sec %ld us\n", diff/1000000, diff%1000000);
    printf("sigwaitinfo() is %ld sec %ld us faster than sigsuspend()\n", diff/1000000, diff%1000000);
    printf("ps. 執行下來，確實通常 sigwaitinfo() 比較快。\n");
}