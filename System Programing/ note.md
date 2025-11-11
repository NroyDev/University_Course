# Signal(Ch.06)
- raise(): 送一個 signal 給自己
    - 行為等價 kill(getpid(), SIGINT)

- signal caught/pending
- signal dispositon(處置): default action (預設)
    - 系統處理 signal 預設會是 default action(可能是kill掉), user 也可以自定義
    - 每個 signal 都有自己的 default action
    - 可透過 sigaction() 自定義 signal disposition
    - 可透過 sigignore() 忽略 signal
- Signals 究竟都有哪些呢？
    - kill -l 可以列出所有的 signal（bash的比較好看）
    - `期末考會考`： `SIGKILL` `SIGSTOP`
        - SIGKILl 和 SIGSTOP 是例外，要擋也擋不住！
    - Ctrl+C: 觸發 SIGINT
    - 系統中允許讓 user define 的 signal: SIGUSR1 SIGUSR2
    - Child process 掛掉的時候會觸發的 signal: SIGCHLD
- signal 被 block 時，是 pending(posted) 的狀態

Signal 示意圖：

    |<-----[pending(posted)]----->|
    v                             v
    |-----------------------------|----> time
    Generate                      Delivered(Caught)

- 自定義 signal handler
    - 一般來說不會有參數或回傳值，但是可以在 sigaction() 透過 siginfo_t 傳遞
- 等待訊號: sigsuspend()
    - 使用傳入的mask，暫停process，等待signal到達，恢復執行
    - 也可以 sigpause(signo) 等待特定 signal，但是
    我看 man 好像建議直接用 sigsuspend()，可能是可攜性有問題，沒那麼通用。
- 設定幾秒後發送signal(SIGALRM): alarm()
    - alarm(int second);        // second 秒後觸發 SIGALRM
    - 如果參數設為 0 => cancel 所有 alarm
- `小考會考`: `alarm()` `signal()` `signal mask` `sigsuspend` `sigaction()`...
    - signal()置換default action
    - signal mask: sigemptyset()...

工業界中長出現這樣的狀況(熱機後固定一段時間做某建事情)：

    |<-init->|<-dt->|<-dt->|<-dt->|<-dt->|<-dt->|
    v  (t1)  v      v      v      v      v      v
    |--------|------|------|------|------|------|----> time

- 可以透過 setitimer() 在 it_value.tv_sec 秒後，固定每 it_interval.tv_sec 秒做alarm()
- 見 **Module 6.pdf, slide 23 of 26**
- pause() Pause整個process直到有個signal signal 它。


# Threads(Ch.07)
- thread 不會共用 stack segement
- 把資料放在 golbal(Data Segment)，在threads間可以共用，但要注意 race condition
- 用 share memory 的方式共享記憶體很方便，後面也常會這樣做，但要注意 race condition。
- pthread UNIX 用來建立 thread 的 library
- pthread_create() 建立一個 thread
- 可以傳遞多參數，彈藥一個結構包起來。
- 處理 race condition => 用鎖
- pthread_mutex_lock() pthread_mutex_trylock() pthread_mutex_unlock()
- sem_wait() sem_trywait() sem_post()
- 假設有五台印表機，印表機是有限資源，使用時必須用鎖鎖起來，如果用mutex管理，使用時候會每台都要問，假設有更多台，會很麻煩 => 所以可以用 semphore 管理
- [參考連結](https://ithelp.ithome.com.tw/articles/10280830)

- compile 的時候要加上 `-lpthread` (有些時候不加會 compile 不過？)

# IPC (Interprocess communication) (Ch.08)
- 我們知道 IPC 前，我們可以透過檔案來傳送檔案
- pipe() 傳輸資料
- mmap 共享記憶體
- 透過網路做跨機器的IPC => Socket 是唯一的解法
- 指令 ipcs ipcrm
- 現在實務上比較常用 memory share
```
        Memory
    ----> (M) <---
   /              \
 ----   ------>  ----
| P1 |  (pipe)  | P2 |
 ----   <------  ----   HOST
   \               /
    ---->(file)<---
        myPipe

        Socket
 ----   ------>   ----
| P1 | (netowrk) | P2 |
 ----   <------   ----
MACHINE 1        MACHINE 2
```

# Short Messages (Ch.09)
- named pipe 叫做 fifo
- 在外面開一個fifo檔案 mkfifo()
- msgget() msgctl()
- SystemV 的 read write 是 msgsnd() msgrcv()

# Shared Memory (Ch.10)
- mmap() 可以把 process 的某一塊 memory 與 file 一對一對應，並且同步
- P1 的一塊記憶體M1 與file同步, P2 的一塊記憶體M2 與 file同步
    - 結果就是 P1的記憶體M1 與 P2的記憶體M2同步
```
 -------    --------
| P1    |  | P2    |
|  m1   |  | m2    |
|  口\  |  | 口     |
 --\--\-   /--/-----
    \  \  /  /
     \  \/  /
      \ /\ /
       \口/
       file
```
- mmap() 也可以當作是一個方便的檔案操作函式 
- 用法示例:
    ```c
        addr = mmap((caddr_t)NULL, statbuf.st_size,PROT_READ, MAP_SHARED, fd, (off_t)0);
    ```
```c
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
    int fd;
    caddr_t addr;
    struct stat statbuf;
    if( argc != 2 ) {
        fprintf(stderr,"Usage: mycat2 filename\n");
        exit(1);
    }
    if( stat(argv[1], &statbuf) == -1 ) {
        perror("stat");
        exit(1);
    }
    fd = open( argv[1], O_RDONLY );
    if (fd == -1) {
        perror("open");
        exit(1);
    }
    addr = mmap((caddr_t)NULL, statbuf.st_size,
    PROT_READ, MAP_SHARED, fd, (off_t)0);
    if (addr == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    /* no longer need fd */
    close(fd);
    write(1, addr, statbuf.st_size);
    return(0);
}
```

- Shareed Memory 考慮：
    - Size ? 共享前必須決定好大小 (file size)
    - Mutex ? 有時候為了防止 race condition，必須 Synchronize
        - 同步手段
        - thread control
        - semaphore (named, unnamed)
        - file control
- 只有 shared memory 可以作到多 process 共享記憶體
- 用 `munmap` 可以取消 `mmap` 造出的共享記憶體
- 用 `ftruncate` 擴大檔案大小 (老師說要記) (給`mmap`用？)
    - 將檔案大小 括大為 pagesize 的六倍
        ```c   
        ftruncate(fd, (off_t)(6 * pagesize));
        ```
- `msync()` 可以用來立即同步 shared memory 的記憶體
    - 雖然基本上會同步，但是因為 `mmap` 依賴於 file，而由於 OS 的設計， disk 通常會累積一段再一同寫入，導致有時不會那麼同步。
 
 - `mmap` 最骨子裡的想法就是透過 _**檔案**_ 共享
 - `mmap` 是 POSIX 的解法，在 SystemV 中，也有 shared memory 的函式，不是 `mmap` 但概念很像
    - 見 Module 10, Slide 9
    - `shmget()` `shmat()` `shmctl()` `shmdt()`
    - Example1(Module 10, Slide 11) 示範了 P1(其中一邊Process)共享記憶體的方式：
        1. 建立足夠大的 file （Line  3 ~ Line 8）
        2. 設定共享          (Line 10 ~ Line 13)
        3. 使用shared memory (Line 15)
        4. 刪掉              (Line 17 Line 18)
        - (另一邊不須建立檔案 只要設定共享即可)
        - Example2 示範shared memory on array
- 有些很舊的系統是用 SystemV 的 shared memory
- POSIX 對應 SystemV
    |         | Open Connection | Set Size      | Attach    | Detach     | Remove rendezvous        |
    | ------- | --------------- | ------------- | --------- | ---------- | ------------------------ |
    | POSIX   | `shm_open()`    | `ftruncate()` | `mmap()`  | `munmap()` | `shm_unlink()`           |
    | SystemV | `shmget()`      | `shmget()`    | `shmat()` | `shmdt()`  | `shmctl() with IPC_RMID` |


---
# Synchronization (Ch.11)
- Process synchronization
    - Signals
    - Record locking (fnctl(2))
    - SystemV semaphores
    - mutex lock
    - reader-writer lock
        - 改善 mutex lock，適合多 reader 單一 writer 的情境，reader 一起讀
    - semaphore
    - condition variable
        - eg. 當共享一個 stack，你搶到了 lock 但不一定有辦法寫(滿了)或讀(空的) => 用 condition variable
- Thread syncronization
    - mutex lock
        ```c
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
        ```
    - reader writer lock
        - 適合多 reader 單一 writer 的情境
        ```c
        pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
        pthread_rwlock_rdlock(&rwlock);     // for reader
        pthread_rwlock_wrlock(&rwlock);     // for writer
        pthread_rwlock_unlock(&rwlock);
        ```
- 小提醒：
    - 在使用 pthread 時，最好在 compile 的時候加上 `-lpthread` 的 link
    - 用 ??? 的時候加上 `-lrt` 的 link

- Condition Variables
    - 不是所有時候，搶到使用權，就能作到想作到的運算
        - eg. 當共享一個 stack，你搶到了 lock 但不一定有辦法寫(滿了)或讀(空的)
    - 搶到使用權，發現不能用 => 釋放出來 (不然會deadlock)
    - mutex lock 和 condition lock 同時出現

```
    ------------        Scheduler        --------------------
    | Runnable |   <----------------->   |      Running     |
    ------------                   ..../ --------------------
      ^                        .../                       |
      | Get mutex lock     .../                           | pthread_cond_wait()
      |                .../ pthread_mutex_lock()          v
    --------------<---/                  --------------------
    | Wait for   | pthread_cond_signal() | Wait for         |
    | mutex lock | <------------------   | condition change |
    --------------                       --------------------

```