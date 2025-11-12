#include<stdio.h>
#include<signal.h>
#define SIGERR_RET ((void(*)(int))-1)
#define SIG_HOLD ((void(*)(int))2)

// On success: returns the previous disposition of sig, or SIG_HOLD
// if sig was previously blocked; on error –1 is returned
void (*sigset(int sig, void (*handler)(int)))(int);
// return 0 on success, or –1 on error
int sighold(int sig);
int sigrelse(int sig);
int sigignore(int sig);
// Always returns –1 with errno set to EINTR
int sigpause(int sig);

// -----------------------------------------------------------------------------

// The sighold() function adds a signal to the process signal mask
int sighold(int sig){
    sigset_t set;

    if(sigemptyset(&set) == -1){
        return -1;
    }
    if(sigaddset(&set, sig) == -1){
        return -1;
    }
    if(sigprocmask(SIG_BLOCK, &set, NULL) == -1){
        return -1;
    }
    return 0;
}

// The sigrelse() function removes a signal from the signal mask.
int sigrelse(int sig){
    sigset_t set;

    if(sigemptyset(&set) == -1){
        return -1;
    }
    if(sigaddset(&set, sig) == -1){
        return -1;
    }
    if(sigprocmask(SIG_UNBLOCK, &set, NULL) == -1){
        return -1;
    }
    return 0;
}

// The sigignore() function sets a signal’s disposition to ignore
int sigignore(int sig){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = SIG_IGN;       // BTW. 我原本以為我要實做這個，但後來做到這邊，查 sigaction 的 man 發現這原本就有 
    if(sigaction(sig, &act, NULL) == -1){
        return -1;
    }
    
    return 0;
}

// The sigpause() function is similar to sigsuspend(), but removes
// just one signal from the process signal mask before suspending the process until
// the arrival of a signal.
int sigpause(int sig){
    sigset_t set, oldset;
    
    if(sigprocmask(SIG_BLOCK, NULL, &oldset) == -1){
        return -1;
    }
    set = oldset;
    if(sigdelset(&set, sig) == -1){
        return -1;
    }

    return sigsuspend(&set);
}

// 課本：
// To establish a signal handler with reliable semantics, System V provided the sigset()
// call (with a prototype similar to that of signal()). As with signal(), the handler argu-
// ment for sigset() can be specified as SIG_IGN, SIG_DFL, or the address of a signal handler.
// Alternatively, it can be specified as SIG_HOLD, to add the signal to the process signal
// mask while leaving the disposition of the signal unchanged.
//
// If handler is specified as anything other than SIG_HOLD, sig is removed from the
// process signal mask (i.e., if sig was blocked, it is unblocked).
//
// 我查 man sigset:
// DESCRIPTION
//    SIG_DFL：  Reset the disposition of sig to the default.
//    SIG_IGN：  Ignore sig.
//    SIG_HOLD： Add  sig to the process's signal mask, but leave the disposition of sig unchanged.
// RETURN VALUE
//    On success, sigset() returns SIG_HOLD if sig  was  blocked  before  the
//    call, or the signal's previous disposition if it was not blocked before
//    the  call.   On  error, sigset() returns -1, with errno set to indicate
//    the error.
void (*sigset(int sig, void (*handler)(int)))(int){
    // check sig is blocked or not
    sigset_t oldset;
    void (*previous_disposition)(int) = NULL;
    if(sigprocmask(SIG_BLOCK, NULL, &oldset) == -1){
        return SIGERR_RET;
    }
    int result = sigismember(&oldset, sig);
    if(result == 1){  // sig has been blcoked
        previous_disposition = SIG_HOLD;
    }else if(result == 0){ // sig is not blocked
        struct sigaction oldact;
        // man sigaction: sigaction() can be called with a NULL second argument to query the current signal handler.
        if(sigaction(sig, NULL, &oldact) == -1){
            return SIGERR_RET;
        }
        previous_disposition = oldact.sa_handler;
    }else{
        return SIGERR_RET;
    }


    if(handler == SIG_DFL){     // If handler is specified as anything other than SIG_HOLD, sig is removed from the process signal mask
        struct sigaction act;
        act.sa_handler = SIG_DFL;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0; // Reliable signals (no SA_RESETHAND or SA_NODEFER)
        if(sigaction(sig, &act, NULL) == -1 || sigrelse(sig) == -1){
            return SIGERR_RET;
        }
    }else if(handler == SIG_IGN){   // If handler is specified as anything other than SIG_HOLD, sig is removed from the process signal mask
        if(sigignore(sig) == -1 || sigrelse(sig) == -1){
            return SIGERR_RET;
        }
    }else if(handler == SIG_HOLD){
        if(sighold(sig) == -1){
            return SIGERR_RET;
        }
    }else{
        struct sigaction act;
        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0; // Reliable signals (no SA_RESETHAND or SA_NODEFER)
        if(sigaction(sig, &act, NULL) == -1 || sigrelse(sig) == -1){
            return SIGERR_RET;
        }
    }

    return previous_disposition;
}

// ps: 這邊的註解大部分都是從課本操下來的 (The Linux Programming Interface - Signals: Advanced Features p.475)