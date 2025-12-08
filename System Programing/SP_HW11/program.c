#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<signal.h>
#define SHM_NAME "SP_HW11_Shared_Memory"

struct Data{
    char message[80];
};

struct Args{
    int d;  // 資料數量
    int r;  // 傳送速率
    int c;  // consumer 數量
    int b;  // buffer size
};

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

void usage(){
    printf("./program [資料數量 D] [傳送速率 R] [consumer 數量 C] [buffer size B]\n");
}

int comsumer_rcv_seqno = -1;
void comsumer_handler(int sig, siginfo_t *info, void *context){
    if(info != NULL){
        comsumer_rcv_seqno = info->si_value.sival_int;
    }else{
        comsumer_rcv_seqno = -1;
    }
    return;
}

void comsumer(const struct Args args, int cid, void* ptr){
    // printf("[Comsumer %d] Starts.\n", cid);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = comsumer_handler;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGUSR1, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }

    unsigned long long recv_cnt = 0;
    while(1){
        pause();
        if(comsumer_rcv_seqno >= 0){
            // printf("[Consumer %d] Handler Recv: %d / Read Shared Memory: %s\n", cid, comsumer_rcv_seqno, ((struct Data*)ptr)[comsumer_rcv_seqno%args.b].message);
            ++recv_cnt;
            int temp = -1;
            sscanf(((struct Data*)ptr)[comsumer_rcv_seqno%args.b].message , "This is message %d", &temp);
            if(temp != comsumer_rcv_seqno){
                --recv_cnt;
            }
        }else if(comsumer_rcv_seqno == -2){
            break;
        }
    }
    // printf("[Comsumer %d] Received %llu times successfully.\n", cid, recv_cnt);
    unsigned long long* start_ptr = (unsigned long long*)((char*)ptr + sizeof(struct Data)*args.b);
    start_ptr[cid] = recv_cnt;
}

void ignore_handler(int sig){
    return;
}
void producer(const struct Args args, void* ptr, pid_t* children_pid){
    printf("[Producer] Starts\n");
    signal(SIGUSR1, ignore_handler);
    for(int i=0;i<args.d;++i){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        int start_time = tv.tv_sec*1000000 + tv.tv_usec;

        struct Data data;
        sprintf(data.message, "This is message %d", i);
        strcpy(&(*(((struct Data*)ptr)+(i%args.b))->message), data.message);

        union sigval value;
        value.sival_int = i;
        // printf("[Producer %d] Send Seq No to all comsumer: %d\n", getpid(), i);
        for(int j=0;j<args.c;++j){
            gettimeofday(&tv, NULL);
            int current_time = tv.tv_sec*1000000 + tv.tv_usec;
            if(current_time - start_time >= args.r){    // 超時了
                // printf("超時\n");
                break;
            }

            int target_pid = children_pid[j];
            if(sigqueue(target_pid, SIGUSR1, value) < 0){
                perror("sigqueue error");
                return;
            }
        }
        gettimeofday(&tv, NULL);
        int current_time = tv.tv_sec*1000000 + tv.tv_usec;
        int sleep_time = args.r - (current_time-start_time);
        if(sleep_time>0){
            usleep(sleep_time);
        }
    }

    // printf("[Producer] Stoping comsumers\n");
    sleep(1);
    union sigval value;
    value.sival_int = -2;
    for(int j=0;j<args.c;++j){
        int target_pid = children_pid[j];
        if(sigqueue(target_pid, SIGUSR1, value) < 0){
            perror("sigqueue error");
            return;
        }
    }
    // printf("[Producer] All comsumers are stopped\n");
}


void do_task(const struct Args args, void* ptr){
    pid_t children_pid[args.c];
    for(int i=0;i<args.c;++i){
        pid_t pid = fork();
        if(pid==-1){
            perror("fork");
            exit(errno);
        }else if(pid==0){
            comsumer(args, i, ptr);
            _exit(0);
        }else{
            children_pid[i] = pid;
        }
    }

    sleep(1);
    producer(args, ptr, children_pid);

    fflush(stdout);
    for(int i=1;i<=args.c;++i){
        wait(NULL);
    }
    unsigned long long cnt = 0;
    unsigned long long* start_ptr = (unsigned long long*)((char*)ptr + sizeof(struct Data)*args.b);
    for(int i=0;i<args.c;++i){
        cnt += start_ptr[i];
    }
    printf("D=%d R=%dus C=%d\n", args.d, args.r, args.c);
    printf("-------------------\n");
    printf("Total messages: %d\nSum of received messages by all consumers: %llu\nLoss rate: %lf\n", args.c*args.d, cnt, 1-((double)cnt/(args.c*args.d)));
    printf("-------------------\n");
}


int main(int argc, const char** argv){
    // process with ARGS
    // ./program [資料數量 D] [傳送速率 R] [consumer 數量 C] [buffer size B]
    if(argc != 5){
        usage();
        exit(-1);
    }
    struct Args args;
    args.d = stoi(argv[1]);
    args.r = stoi(argv[2]);     // 這裡問過助教了 可以改成 micro second (因為milisecond看不太出來loss rate的變化)
    args.c = stoi(argv[3]);
    args.b = stoi(argv[4]);


    // Open Shared memory
    const int SIZE = args.b*sizeof(struct Data) + args.c*sizeof(unsigned long long);
    int shmfd = shm_open(SHM_NAME, O_CREAT|O_RDWR, 0600);
    if(shmfd == -1){
        perror("shm_open");
        exit(errno);
    }
    if(ftruncate(shmfd, SIZE) == -1){
        perror("ftruncate");
        exit(errno);
    }
    void* ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(ptr == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }
    memset(ptr, 0, SIZE);

    do_task(args, ptr);

    // Close Shared memory
    munmap(ptr, SIZE);
    close(shmfd);
    if(shm_unlink(SHM_NAME) == -1){
        perror("shm_unlink");
        exit(-1);
    }
}