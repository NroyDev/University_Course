#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <pwd.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdlib.h>

/*
    1. 根據傳近來的argv 轉換成完整的絕對路徑
    2. 讀/proc資料夾 得到所有的process資訊
    3. => 對每個process的fd readlink() 比對是否跟我們要的相符
    4. => 如果相符 則print出process的名字(從/proc/PID/status讀)
*/

void usage(const char** argv){
    fprintf(stdout, "Usage: %s [filepath]\n", argv[0]);
    return;
}

int main(int argc, const char** argv){
    if(argc != 2){
        usage(argv);
        exit(-1);
    }

    // 根據傳近來的argv 轉換成完整的絕對路徑
    char path[PATH_MAX];
    if(realpath(argv[1], path) == NULL){
        perror("Error - realpath");
        exit(errno);
    }
    fprintf(stdout, "Processes that open '%s':\n", path);

    // 讀/proc資料夾 得到所有的process資訊
    // ----- open /proc -----
    const char* path_proc = "/proc";
    DIR* dir_proc = opendir(path_proc);
    if(dir_proc == NULL){
        perror("Error - opendir");
        exit(errno);
    }
    // ----- read /proc -----
    int isOpendby_process = 0;
    char path_PID[PATH_MAX] = {};
    char path_PID_fd[PATH_MAX] = {};
    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE] = {};
    const struct dirent* dirent_proc = NULL;
    errno = 0;
    while((dirent_proc = readdir(dir_proc)) != NULL){
        if(!('0' <= dirent_proc->d_name[0] && dirent_proc->d_name[0] <= '9')){
            continue;
        }
        int isOpen_target = 0;
        // 對每個process的fd readlink() 比對是否跟我們要的相符
        // ----- open /proc/PID/fd -----
        strcpy(path_PID,path_proc);
        strcat(path_PID,"/");
        strcat(path_PID,dirent_proc->d_name);
        strcat(path_PID,"/fd");
        DIR* dir_proc_fd = opendir(path_PID);
        if(dir_proc_fd == NULL){
            errno = 0;
            continue;
        }
        // ----- read /proc/PID/fd -----
        const struct dirent* dirent_proc_fd = NULL;
        errno = 0;
        while((dirent_proc_fd = readdir(dir_proc_fd)) != NULL){
            strcpy(path_PID_fd, path_PID);
            strcat(path_PID_fd,"/");
            strcat(path_PID_fd, dirent_proc_fd->d_name);
            const int len = readlink(path_PID_fd, buf, BUF_SIZE);
            if(len == -1){
                // may no longer exist or permission denied
                errno = 0;
                continue;
            }
            buf[len] = '\0';
            if(strcmp(buf, path) == 0){         // 找到了！！！
                isOpen_target = 1;
                break;
            }
        }
        if(errno != 0){
            perror("Error - readdir");
            exit(errno);
        }
        // ----- close /proc/PID/fd -----
        if(closedir(dir_proc_fd) == -1){
            perror("Error - closedir");
            exit(errno);
        }


        // 如果相符 則print出process的名字(從/proc/PID/status讀)
        if(isOpen_target){  // read name
            isOpendby_process = 1;
            strcpy(path_PID,path_proc);
            strcat(path_PID,"/");
            strcat(path_PID,dirent_proc->d_name);
            strcat(path_PID,"/status");
            FILE* fp = fopen(path_PID, "r");
            if(fp == NULL){
                fprintf(stdout, "Warning - %s cannot open (may not exist)\n", path_PID);
                continue;
            }

            while(fscanf(fp,"%s",buf) != EOF){
                if(strspn(buf,"Name:") == 5){   // found name
                    if(fgets(buf,BUF_SIZE,fp) == NULL){
                        perror("Error - fgets failed to read Name field");
                        exit(errno);
                    }
                    if(buf[strlen(buf)-1] == '\n'){
                        buf[strlen(buf)-1] = '\0';
                    }
                    fprintf(stdout, "\t%s (%s)\n", buf+1, dirent_proc->d_name);
                }
            }
            // -------- close /proc/PID/status --------
            if(fclose(fp) == EOF){
                perror("Error - fclose");
                exit(errno);
            }
        }
    }
    if(errno != 0){
        perror("Error - readdir");
        exit(errno);
    }

    if(!isOpendby_process){
        fprintf(stdout, "No process is opening %s.\n", path);
    }
    
    // ----- close /proc -----
    if(closedir(dir_proc) == -1){
        perror("Error - closedir");
        exit(errno);
    }
}