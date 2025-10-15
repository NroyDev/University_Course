#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <pwd.h>
#include <stdint.h>
#include <unistd.h>

void usage(const char** argv){
    fprintf(stdout, "Usage: %s [username]\n", argv[0]);
    return;
}

uid_t getUID(int argc, const char** argv){
    if(argc != 2){
        usage(argv);
        exit(-1);
    }
    struct passwd pwd;
    struct passwd *result;
    char *buf;
    long bufsize;

    if((bufsize = sysconf(_SC_GETPW_R_SIZE_MAX)) == -1){
        bufsize = 16384;
    }
    if((buf = (char*)malloc(bufsize)) == NULL){
        perror("malloc");
        exit(-2);
    }

    int s = getpwnam_r(argv[1], &pwd, buf, bufsize, &result);
    if(result == NULL){
        if(s == 0){
            printf("Not found\n");
        }else{
            errno = s;
            perror("getpwnam_r");
        }
        exit(errno);
    }

    uid_t ret = pwd.pw_uid;
    free(buf);
    return ret;
}

int main(int argc, const char** argv){
    // ----------------------------- get target UID -----------------------------
    uid_t uid_target = getUID(argc, argv);
    fprintf(stdout, "Process Run by %s (%d)\n", argv[1], uid_target);
    fprintf(stdout, "%-20s %s\n", "Process Name", "PID");
    fprintf(stdout, "%-20s %s\n", "-------------------","---");
    // ----------------------------- open /proc -----------------------------
    const char* path_proc = "/proc";
    DIR* dir_proc = opendir(path_proc);
    if(dir_proc == NULL){
        perror("Error - opendir");
        exit(errno);
    }


    // ----------------------------- read /proc -----------------------------
    const int MAX_PATH = 1024;
    char path_PID[MAX_PATH] = {};
    struct dirent* dirent_proc = NULL;
    errno = 0;
    while((dirent_proc = readdir(dir_proc)) != NULL){
        if(!('0' <= dirent_proc->d_name[0] && dirent_proc->d_name[0] <= '9')){
            continue;
        }

        // -------- open /proc/PID/status --------
        strcpy(path_PID,path_proc);
        strcat(path_PID,"/");
        strcat(path_PID,dirent_proc->d_name);
        strcat(path_PID,"/status");
        FILE* fp = fopen(path_PID, "r");
        if(fp == NULL){
            fprintf(stdout, "Warning - %s cannot open (may not exist)\n", path_PID);
            continue;
        }

        // -------- read /proc/PID/status --------
        const int BUF_SIZE = 4096;
        char buf[BUF_SIZE];
        char name[BUF_SIZE];
        int is_target_user = 0;     // flag = true if the process is run by target user
        while(fscanf(fp,"%s",buf) != EOF){
            if(strspn(buf,"Name:") == 5){   // found name
                if(fgets(buf,BUF_SIZE,fp) == NULL){
                    perror("Error - fgets failed to read Name field");
                    exit(errno);
                }
                if(buf[strlen(buf)-1] == '\n'){
                    buf[strlen(buf)-1] = '\0';
                }
                strcpy(name, buf+1);
            }else if(strspn(buf, "Uid:") == 4){ //found uid
                uid_t real_uid = 0;
                if(fscanf(fp, "%u", &real_uid)==EOF){
                    perror("Error - fscanf");
                    exit(errno);
                }
                if(fgets(buf,BUF_SIZE,fp) == NULL){
                    perror("Error - fgets failed to read Name field");
                    exit(errno);
                }
                if(real_uid == uid_target){
                    is_target_user = 1;
                }
            }
        }
        if(is_target_user){ // output if is target
            fprintf(stdout,"%-20s %s\n", name, dirent_proc->d_name);
        }

        // -------- close /proc/PID/status --------
        if(fclose(fp) == EOF){
            perror("Error - fclose");
            exit(errno);
        }
    }
    if(errno != 0){
        perror("Error - readdir");
        exit(errno);
    }
    
    // ----------------------------- close /proc -----------------------------
    if(closedir(dir_proc) == -1){
        perror("Error - closedir");
        exit(errno);
    }
}