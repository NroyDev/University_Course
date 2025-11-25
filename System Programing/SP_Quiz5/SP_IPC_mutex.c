#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/mman.h>
#include<pthread.h>
#include<sys/wait.h>

#define B_FALSE 0
#define B_TRUE 1
#define NUM_EACH_PROC 100
#define SHARED_DATA_FILE "./proc_mutex.dat"     /* Shared data is created in this mmapped file. */
typedef struct globalStuff{
    pthread_mutex_t sharedMutex;
    int count;
}globalStuff;
struct globalStuff * globalArea;

void myfork(int);
int stoi(const char*);



// 可以在這邊關掉 lock
int DISABLE_LOCK = B_FALSE;        // B_TRUE B_FALSE
int main(int argc, const char** argv){
    if(!(2<=argc && argc<=3)){
        printf("%s [num of proces] [(Optional) DISABLE_LOCK (0/1)]\n", argv[0]);
        printf("example: \n\t%s 1000\n", argv[0]);
        exit(-1);
    }
    int n = stoi(argv[1]);
    if(argc == 3 && stoi(argv[2]) >0){
        DISABLE_LOCK = B_TRUE;
    }

    myfork(n);

    // ---------------------------------------------------------------------------------------
    int shared_fd = open(SHARED_DATA_FILE, O_RDWR, 0777);
    if(shared_fd < 0){
        perror("Error opening shared mutex area");
        exit(errno);
    }
    globalArea = (struct globalStuff *)mmap(NULL, sizeof(struct globalStuff), PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);
    if(globalArea == (struct globalStuff *)-1){
        perror("Error getting shared Mutex virtual addr");
        exit(errno);
    }
    pthread_mutex_lock(&(globalArea->sharedMutex));
    printf("-----------------------------------------\n");
    if(DISABLE_LOCK){
        printf("沒有 MUTEX LOCK\n");
    }else{
        printf("有 MUTEX LOCK\n");
    }
    printf("理論上 golbalArea share 的 count: %d\n", NUM_EACH_PROC*n);
    printf("最後在 golbalArea share 的 count: %d\n", (globalArea->count));
    printf("-----------------------------------------\n");
    pthread_mutex_unlock(&(globalArea->sharedMutex));
    system("rm ./proc_mutex.dat");
}




// ########################################################
//  
// ########################################################

void proc_task(int id){
    /* For interprocess mutex init. */
    pthread_mutexattr_t mutex_attributes;
    /* True if we set up shared area. */
    int createdHere = B_FALSE;

    /* Open the shared area. Err out if it already exists 
    (which means we are not the first process to use it, and thus not to be the creator if it. */
    int shared_fd = open(SHARED_DATA_FILE, (O_RDWR |O_CREAT | O_EXCL), 0777);
    /* Flag that we are the creator if we are, or just open the file otherwise. */
    if((shared_fd == -1) && (errno == EEXIST)) {
        shared_fd = open(SHARED_DATA_FILE, O_RDWR, 0777);
    }else{
        createdHere = B_TRUE;
    }
    /* Some other error opening the file. */
    if(shared_fd < 0){
        perror("Error opening shared mutex area");
        exit(errno);
    }

    /* Expand the file to hold the shared data, before mmapping it. */
    if(createdHere){
        if(ftruncate(shared_fd, sizeof(struct globalStuff))){
            perror("ftruncate");
            exit(errno);
        }
    }
    /* Mmap the file so all processes needing access to it can see it. */
    globalArea = (struct globalStuff *)mmap(NULL, sizeof(struct globalStuff), PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0);
    if(globalArea == (struct globalStuff *)-1){
        perror("Error getting shared Mutex virtual addr");
        exit(errno);
    }

    /* Initialize mutex in shared region if we are the shared region creator. */
    if(createdHere){
        /* Initialize mutex attributes list. */
        if(pthread_mutexattr_init(&mutex_attributes)){
            perror("pthread_mutexattr_init");
            exit(errno);
        }
        /* Reflect in the attributes that this is an interprocess shared mutex. */
        if(pthread_mutexattr_setpshared(&mutex_attributes, PTHREAD_PROCESS_SHARED)) {
            perror("pthread_mutexattr_setpshared");
            exit(errno);
        }
        /* Initialize the mutex in the mmapped area, with the attributes above. */
        if(pthread_mutex_init(&globalArea->sharedMutex, &mutex_attributes)) {
            perror("pthread_mutex_init");
            exit(errno);
        }
    }

    printf("Process %d Ready.\n", id);
    sleep(2);
    if(!DISABLE_LOCK){
        pthread_mutex_lock(&(globalArea->sharedMutex));
        printf("Process %d GET Lock\n", id);
    }
    printf("Process %d: Before %d", id, globalArea->count);
    for(int i=0;i<NUM_EACH_PROC;++i){
        ++(globalArea->count);
    }
    printf(" After %d\n", globalArea->count);
    printf("Process %d completed the work\n", id);
    printf("--------------------------------\n");
    if(!DISABLE_LOCK){
        printf("Process %d RELEASE Lock\n", id);
        fflush(stdout);
        pthread_mutex_unlock(&(globalArea->sharedMutex));
    }
}

void myfork(int n){
    for(int i=1;i<=n;++i){
        switch(fork()){
        case -1:
            perror("fork");
            exit(-1);
        case 0:
            proc_task(i);
            _exit(0);
        
        default:
            break;
        }
    }

    // wait
    for(int i=1;i<=n;++i){
        wait(NULL);
    }
}

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