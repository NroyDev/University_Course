#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>

struct SharedStack{
    sem_t mutex;  //=1 protect top index
    sem_t empty_slots;// = m;
    sem_t full_slots;// = 0;
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
    if(sem_wait(&(header->empty_slots))==-1 || sem_wait(&(header->mutex))){
        perror("sem_wait");
        exit(-1);
    }

    header->full_top += 1;
    buffer[header->full_top].pid  = data.pid;
    buffer[header->full_top].data = data.data;

    if(sem_post(&(header->mutex)) == -1 || sem_post(&(header->full_slots)) == -1){
        perror("sem_post");
        exit(-1);
    }
}

struct Element pop(void* ptr){
    struct SharedStack* header = ptr;
    struct Element* buffer = (struct Element*)((char*)ptr + sizeof(struct SharedStack));
    if(sem_wait(&(header->full_slots))==-1 || sem_wait(&(header->mutex))){
        perror("sem_wait");
        exit(-1);
    }

    struct Element ret;
    ret.pid  = buffer[header->full_top].pid;
    ret.data = buffer[header->full_top].data;
    header->full_top -= 1;

    if(sem_post(&(header->mutex)) == -1 || sem_post(&(header->empty_slots)) == -1){
        perror("sem_post");
        exit(-1);
    }

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
    // int* buffer = ptr+sizeof(header);
    if(sem_init(&(header->mutex), (m+1), 1) == -1 || sem_init(&(header->empty_slots), (m+1), m) == -1 || sem_init(&(header->full_slots), (m+1), 0)==-1){
        perror("sem_init");
        exit(-1);
    }
    header->full_top = -1;

    fork_things(ptr);

    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        exit(-1);
    }
}