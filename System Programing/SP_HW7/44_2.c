#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/wait.h>

#define MAX_POPEN_FD 4096
pid_t fdpid_table[MAX_POPEN_FD] = {};     // fdpid_table[fd] to get pid of open fd


FILE* mypopen(const char *cmd, const char *type){
    // -------------------- type pipe --------------------
    if(strcmp(type, "r") != 0 && strcmp(type, "w")){  // invalid type
        errno = EINVAL;
        return NULL;
    }
    int pipefd[2];
    if(pipe(pipefd) == -1){
        return NULL;
    }
    int pipe_parent = pipefd[0];;
    int pipe_child  = pipefd[1];;
    if(type[0] == 'w'){
        pipe_parent = pipefd[1];
        pipe_child  = pipefd[0];
    }

    // -------------------- fork exec --------------------
    pid_t pid = fork();
    switch (pid){
    case -1:
        int errno_backup = errno;
        close(pipe_parent);
        close(pipe_child);
        errno = errno_backup;
        return NULL;
    
    case 0:
        // child
        // close unused fd
        if(close(pipe_parent) == -1){
            perror("Error - close() 1");
            _exit(errno);
        }
        // 44-2題目的要求 This structure will also assist with the SUSv3 requirement that any still-open file streams created by earlier calls to popen() must be closed in the new child process.
        for(int i=0;i<MAX_POPEN_FD;++i){        // 防止parent一直popen 導致child hold其他child的pipe fd 導致pipe關不起來
            if(i != pipe_child && fdpid_table[i] > 0 && close(i) == -1){
                perror("Error - close() 2");
                _exit(errno);
            }
        }

        // redirect
        if(type[0]=='w' && pipe_child != STDIN_FILENO){
            if(dup2(pipe_child, STDIN_FILENO) == -1){
                perror("Error - dup2() 1");
                _exit(errno);
            }
            if(close(pipe_child) == -1){
                perror("Error - close() 3");
                _exit(errno);
            }
        }else if(type[0] == 'r' && pipe_child != STDOUT_FILENO){
            if(dup2(pipe_child, STDOUT_FILENO) == -1){
                perror("Error - dup2() 2");
                _exit(errno);
            }
            if(close(pipe_child) == -1){
                perror("Error - close() 4");
                _exit(errno);
            }
        }
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        // should not be exec below
        perror("ERROR - execel");
        _exit(errno);

    default:
        // close unused fd
        if(close(pipe_child) == -1){
            int errno_backup = errno;
            close(pipe_parent);
            waitpid(pid, NULL, 0);  // 回收child process
            errno = errno_backup;
            return NULL;
        }

        FILE* fp = fdopen(pipe_parent, type);
        if(fp == NULL){
            int errno_backup = errno;
            close(pipe_parent);
            waitpid(pid, NULL, 0);
            errno = errno_backup;
            return NULL;
        }
        int fd = fileno(fp); 
        if(fd < 0 || fd >= MAX_POPEN_FD){
            int errno_backup = errno;
            fclose(fp);
            waitpid(pid, NULL, 0);
            errno = errno_backup;
            return NULL;
        }
        fdpid_table[fd] = pid;

        return fp;
    }
}

int mypclose(FILE* stream){
    if(stream == NULL){
        errno = EINVAL;
        return -1;
    }

    int fd = fileno(stream);
    if(!(0 <= fd && fd < MAX_POPEN_FD) || fdpid_table[fd] == 0){  // fail or out of range or not by popen
        errno = EINVAL;
        return -1;
    }
    pid_t pid = fdpid_table[fd];
    fdpid_table[fd] = 0;

    if(fclose(stream) != 0){
        waitpid(pid, NULL, 0);
        return -1;
    }

    int status = -1;
    int temp;
    do{
        errno = 0;
    }while((temp=waitpid(pid, &status, 0)) == -1 && errno == EINTR);    // 防只其他signal打斷
    if(temp == -1){
        return -1;
    }

    if(WIFEXITED(status)){
        return WEXITSTATUS(status);
    }
    return -1;
}

int main(){
    FILE* fp;
    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE];
    
    printf("popen(\"ls -l\", \"r\") \n");
    if((fp = mypopen("ls -l", "r")) == NULL){
        perror("ERROR - mypopen()");
        exit(errno);
    }
    printf("read from fp(ls -l):\n");
    while(fgets(buf, BUF_SIZE, fp) != NULL){
        printf("%s", buf);
    }
    printf("end of read\n");
    printf("pclose ls -l\n");
    printf("ls -l exit, sataus = %d\n", mypclose(fp));


    printf("--------------------------------------------------------------------------------\n");
    printf("popen(\"tee ./tmp.txt\", \"w\") \n");
    if((fp = mypopen("tee ./tmp.txt", "w")) == NULL){
        perror("ERROR - mypopen()");
        exit(errno);
    }
    printf("write to fp(tee ./tmp.txt):\n");
    fprintf(stdout  , "Hello world\n");
    fprintf(fp      , "Hello world\n");
    fprintf(stdout  , "CSE :)\n");
    fprintf(fp      , "CSE :)\n");
    fprintf(stdout  , "this is a testing message.\n");
    fprintf(fp      , "this is a testing message.\n");
    fprintf(stdout  , "HaHa\n");
    fprintf(fp      , "HaHa\n");
    printf("end of write\n");
    printf("pclose tee ./tmp.txt\n");
    printf("tee ./tmp.txt exit, sataus = %d\n", mypclose(fp));
    

    printf("--------------------------------------------------------------------------------\n");
    FILE* fp2, *fp3;
    printf("popen(\"sleep 3\", \"r\") \n");
    if((fp = mypopen("sleep 3", "r")) == NULL){
        perror("ERROR - mypopen()");
        exit(errno);
    }
    printf("popen(\"cat ./tmp.txt\", \"r\") \n");
    if((fp2 = mypopen("cat ./tmp.txt", "r")) == NULL){
        perror("ERROR - mypopen()");
        exit(errno);
    }
    printf("popen(\"grep \"test\"\", \"r\") \n");
    if((fp3 = mypopen("grep \"test\"", "w")) == NULL){
        perror("ERROR - mypopen()");
        exit(errno);
    }
    printf("\n");

    printf("read from fp2(cat ./tmp.txt):\n");
    while(fgets(buf, BUF_SIZE, fp2) != NULL){
        printf("%s", buf);
    }
    printf("end of read\n");
    printf("\n");
    printf("write to fp3(grep \"test\"):\n");
    fprintf(stdout  , "Hello world\n");
    fprintf(fp3     , "Hello world\n");
    fprintf(stdout  , "CSE :)\n");
    fprintf(fp3     , "CSE :)\n");
    fprintf(stdout  , "this is a testing message.\n");
    fprintf(fp3     , "this is a testing message.\n");
    fprintf(stdout  , "HaHa\n");
    fprintf(fp3     , "HaHa\n");
    printf("end of write\n");
    printf("\n");


    printf("pclose cat ./tmp.txt\n");
    printf("=> cat ./tmp.txt exit, sataus = %d\n", mypclose(fp2));
    printf("pclose grep \"test\"\n");
    printf("=> grep \"test\" exit, sataus = %d\n", mypclose(fp3));
    printf("pclose tee sleep 3\n");
    printf("=> sleep 3 exit, sataus = %d\n", mypclose(fp));


    return 0;
}