#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/shm.h>    // SystemV
#include <sys/sem.h>    // SystemV
#include <sys/wait.h>

union semun{        // 這邊從 man semctl 中 copy下來的
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
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

int myget_sem(){
    key_t key = ftok(".", 'S');   // semaphore mutex empty_slots full_slots
    if(key == -1){
        perror("ftok");
        exit(-1);
    }
    int semid = semget(key, 0, 0);
    if(semid == -1){
        perror("semget");
        exit(-1);
    }
    return semid;
}

int mysem_wait(int semid, int semno){
    struct sembuf sb;
    sb.sem_num = semno;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    return semop(semid, &sb, 1);    // 1是做1次
}
int mysem_post(int semid, int semno){
    struct sembuf sb;
    sb.sem_num = semno;
    sb.sem_op = 1;
    sb.sem_flg = 0;
    return semop(semid, &sb, 1);    // 1是做1次
}

void push(struct Element data, void* ptr){
    int semid = myget_sem();    // 0=>mutex 1=>empty 2=>full
    const int mutex = 0;
    const int empty = 1;
    const int full = 2;

    int* write_index = (int*)ptr;
    struct Element* buffer = (struct Element*)((char*)ptr + sizeof(int));
    if(mysem_wait(semid, empty)==-1 || mysem_wait(semid, mutex)==-1){
        perror("mysem_wait");
        exit(-1);
    }

    (*write_index) += 1;
    buffer[(*write_index)].pid  = data.pid;
    buffer[(*write_index)].data = data.data;

    if(mysem_post(semid, mutex) == -1 || mysem_post(semid, full) == -1){
        perror("mysem_post");
        exit(-1);
    }
}

struct Element pop(void* ptr){
    int semid = myget_sem();    // 0=>mutex 1=>empty 2=>full
    const int mutex = 0;
    const int empty = 1;
    const int full = 2;

    int* write_index = (int*)ptr;
    struct Element* buffer = (struct Element*)((char*)ptr + sizeof(int));
    if(mysem_wait(semid, full)==-1 || mysem_wait(semid, mutex)==-1){
        perror("mysem_wait");
        exit(-1);
    }

    struct Element ret;
    ret.pid  = buffer[(*write_index)].pid;
    ret.data = buffer[(*write_index)].data;
    (*write_index) -= 1;

    if(mysem_post(semid, mutex) == -1 || mysem_post(semid, empty) == -1){
        perror("mysem_post");
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
    // printf("# Producer P%d (PID %d) end writing.\n", id, getpid());
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

    // wait
    for(int i=1;i<=m;++i){
        wait(NULL);
    }
}



int main(int argc, const char** argv){
    // args
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

    // open shared memory
    const int SIZE = sizeof(int) + m*sizeof(struct Element);
    key_t key = ftok(".", 'A');
    if(key == -1){
        perror("ftok");
        exit(-1);
    }
    int shmid = shmget(key, SIZE, IPC_CREAT | 0666);
    if(shmid == -1){
        perror("shmget");
        exit(-1);
    }
    void* ptr = shmat(shmid, NULL, 0);
    if(ptr == (void*)-1){
        perror("shmat");
        exit(-1);
    }

    // initalization
    memset(ptr, 0, SIZE);
    int* write_index = (int*)ptr;
    key = ftok(".", 'S');   // semaphore mutex empty_slots full_slots
    if(key == -1){
        perror("ftok");
        exit(-1);
    }
    int semid = semget(key, 3, IPC_CREAT | 0666);       // 3 semaphores
    if(semid == -1){
        perror("semget");
        exit(-1);
    }
    union semun temp;
    temp.val = 1;
    if(semctl(semid, 0, SETVAL, temp) == -1){   // mutex
        perror("semctl mutex");
        semctl(semid, 0, IPC_RMID);
        exit(-1);
    }
    temp.val = m;
    if(semctl(semid, 1, SETVAL, temp) == -1){   // empty_slots
        perror("semctl empty_slots");
        semctl(semid, 1, IPC_RMID);
        exit(-1);
    }
    temp.val = 0;
    if(semctl(semid, 2, SETVAL, temp) == -1){   // full_slots
        perror("semctl full_slots");
        semctl(semid, 2, IPC_RMID);
        exit(-1);
    }
    (*write_index) = -1;


    // DO
    fork_things(ptr);


    // recycle
    semctl(semid, 0, IPC_RMID);
    semctl(semid, 1, IPC_RMID);
    semctl(semid, 2, IPC_RMID);
    shmdt(ptr);
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)NULL);
}