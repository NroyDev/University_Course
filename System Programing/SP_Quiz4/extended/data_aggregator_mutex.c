#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

struct SharedStack{
    pthread_mutex_t mutex;  //=1 protect top index
    int empty_slots;// = m;
    int full_slots;// = 0;
    int full_top;
};

struct Element{
    int pid;
    int data;
};

int m = 0;  // #Producer
int n = 0;  // #Write

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


void push(struct Element data, void* ptr){
    struct SharedStack* header = ptr;
    struct Element* buffer = (struct Element*)((char*)ptr + sizeof(struct SharedStack));
    while(1){
        pthread_mutex_lock(&(header->mutex));
        if(header->empty_slots > 0){
            --(header->empty_slots);
            break;
        }else{
            pthread_mutex_unlock(&(header->mutex));
        }
    }

    header->full_top += 1;
    buffer[header->full_top].pid  = data.pid;
    buffer[header->full_top].data = data.data;

    ++(header->full_slots);
    pthread_mutex_unlock(&(header->mutex));
}

struct Element pop(void* ptr){
    struct SharedStack* header = ptr;
    struct Element* buffer = (struct Element*)((char*)ptr + sizeof(struct SharedStack));
    while(1){
        pthread_mutex_lock(&(header->mutex));
        if(header->full_slots > 0){
            --(header->full_slots);
            break;
        }else{
            pthread_mutex_unlock(&(header->mutex));
        }
    }
    

    struct Element ret;
    ret.pid  = buffer[header->full_top].pid;
    ret.data = buffer[header->full_top].data;
    header->full_top -= 1;

    ++(header->empty_slots);
    pthread_mutex_unlock(&(header->mutex));

    return ret;
}

void comsumer(void* ptr){
    printf("# Comsumer (PID %d) waits on Semaphore 1 (Full Count)\n", getpid());
    int sum = 0;
    for(int i=0;i<m*n;++i){
        struct Element r = pop(ptr);
        sum += r.data;
        printf("# Comsumer reads (%d, %d). Sum = %d.\n", r.pid, r.data, sum);
    }
    printf("# Output:\n");
    printf("Total Data Points Processed: %d\n", n*m);
    printf("Final Cumulative Sum: %d\n", sum);
}

void producer(int id, void* ptr){
    printf("# Producer P%d (PID %d) starts writing...\n", id, getpid());
    srand(id);
    for(int i=0;i<n;++i){
        int randVal = rand()%10+1;  // 1-10
        struct Element temp;
        temp.pid = getpid();
        temp.data = randVal;
        push(temp, ptr);
        printf("# P%d writes (%d, %d), increase Sem 1.\n", id, getpid(), randVal);
    }
}


void fork_things(void* ptr){
    // Producers
    for(int i=1;i<=m;++i){
        switch (fork()){
        case -1:
            perror("fork");
            exit(-1);
            break;
        case 0:
            producer(i, ptr);
            _exit(0);
        
        default:
            break;
        }
    }

    // Comsumer
    comsumer(ptr);
}



int main(int argc, const char** argv){
    if(argc != 3){
        fprintf(stderr, "Usage: %s #Producer #Write\n", argv[0]);
        exit(-1);
    }
    m = stoi(argv[1]);
    n = stoi(argv[2]);
    if(m<0||n<0){
        fprintf(stderr, "can't be negative\n");
        fprintf(stderr, "Usage: %s #Producer #Write\n", argv[0]);
        exit(-1);
    }

    const int SIZE = sizeof(struct SharedStack) + m*sizeof(struct Element);
    const char *name = "SP_Quiz4";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("shm_open");
        exit(-1);
    }
    if(ftruncate(shm_fd,SIZE) == -1){
        perror("ftruncate");
        exit(-1);
    }
    void* ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(ptr == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }

    memset(ptr, 0, SIZE);
    struct SharedStack* header = ptr;
    pthread_mutexattr_t mutex_attr;
    if(pthread_mutexattr_init(&mutex_attr) != 0){
        perror("pthread_mutexattr_init");
    }
    if(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) != 0){
        perror("pthread_mutexattr_setpshared");
        pthread_mutexattr_destroy(&mutex_attr);
        exit(-1);
    }
    if(pthread_mutex_init(&(header->mutex), &mutex_attr) != 0){
        perror("pthread_mutex_init");
        pthread_mutexattr_destroy(&mutex_attr);
        exit(-1);
    }
    header->full_slots = 0;
    header->empty_slots = m;
    header->full_top = -1;

    fork_things(ptr);

    if(munmap(ptr, SIZE) == -1){
        perror("munmap");
        exit(-1);
    }
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        exit(-1);
    }
    pthread_mutexattr_destroy(&mutex_attr);
}