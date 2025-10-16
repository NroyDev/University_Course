#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 4096
#define NMAX_CHILDREN 512
#define NMAX_PREFIX 8192

struct Process{
    char* name;
    pid_t pid;
    pid_t ppid;
    pid_t cpid[NMAX_CHILDREN];  // pid of children process
    unsigned int nchildren;     // number of children process
};
struct Process** process_table;

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

// return PID_MAX which is maxval of PID
int getPID_MAX(){
    // ------------------------------ read pid_max ------------------------------
    FILE* fp_pid_max = fopen("/proc/sys/kernel/pid_max","r");
    if(fp_pid_max == NULL){
        perror("Error - /proc/sys/kernel/pid_max open failed");
        exit(errno);
    }
    int PID_MAX;
    if(fscanf(fp_pid_max, "%d", &PID_MAX) == EOF && errno != 0){
        perror("Error - fscanf");
        exit(errno);
    }
    if(fclose(fp_pid_max) == EOF){
        perror("Error - fclose");
        exit(errno);
    }
    return PID_MAX;
}

// read from /proc, and set information into process_table
// include pid ppid cid name
void setProcess_table(struct Process** process_table, const int PID_MAX){
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

        // -------- convert PID string to long --------
        uid_t pid = stoi(dirent_proc->d_name);
        
        // -------- alloc space --------
        process_table[pid] = (struct Process*)malloc(sizeof(struct Process));
        process_table[pid]->name = NULL;
        process_table[pid]->pid = pid;
        process_table[pid]->ppid = 0;
        process_table[pid]->nchildren = 0;

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
        char buf[BUF_SIZE];
        while(fgets(buf, BUF_SIZE, fp)){
            if(strspn(buf,"Name:") == 5){   // found name
                if(buf[strlen(buf)-1] == '\n'){
                    buf[strlen(buf)-1] = '\0';
                }
                process_table[pid]->name = (char*)malloc(sizeof(char)*(strlen(buf+5+1)+5+1));
                strcpy(process_table[pid]->name, buf+5+1);
            }else if(strspn(buf,"PPid:") == 5){
                if(buf[strlen(buf)-1] == '\n'){
                    buf[strlen(buf)-1] = '\0';
                }
                process_table[pid]->ppid = stoi(buf+5+1);
            }
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



    // ----------------------------- Update Children pointer -----------------------------
    for(int i=0;i<=PID_MAX;++i){
        struct Process* current = process_table[i];
        if(current != NULL && process_table[current->ppid] != NULL){
            struct Process* parent = process_table[current->ppid];
            if(parent->nchildren >= NMAX_CHILDREN){
                fprintf(stderr, "Error - Exceed NMAX_CHILDREN\n");
                exit(-1);
            }
            parent->cpid[(parent->nchildren++)] = i;
        }
    }
}

// print parent-child relation tree
void print(uid_t pid, char* prefix){
    struct Process* current_process = process_table[pid];
    if(current_process == NULL){
        fprintf(stderr, "Warning - going to invalid pointer in process_table[%d]\n", pid);
        return;
    }
    // ------------------ print proc name ------------------
    fprintf(stdout, "%s", current_process->name);

    // ------------------ set prefix for next level ------------------
    int prefix_size = strlen(prefix);
    const int name_size = strlen(current_process->name);
    if(name_size+prefix_size+3+1>NMAX_PREFIX){
        fprintf(stderr, "Error - prefix out of range\n");
        exit(-1);
    }
    if(current_process->nchildren >= 1){
        for(int i=0;i<name_size;++i){
            prefix[prefix_size+i] = ' ';
        }
        prefix[prefix_size+name_size+0] = ' ';
        if(current_process->nchildren == 1){
            prefix[prefix_size+name_size+1] = ' ';
        }else{
            prefix[prefix_size+name_size+1] = '|';
        }
        prefix[prefix_size+name_size+2] = ' ';
        prefix[prefix_size+name_size+3] = '\0';
    }
    prefix_size += name_size + 3;   // update size

    // ------------------ draw ------------------
    if(current_process->nchildren == 0){    // 沒有更多子process => 直接輸出換行
        fprintf(stdout, "\n");
    }else if(current_process->nchildren == 1){
        fprintf(stdout, "───");
        print(current_process->cpid[0], prefix);
    }else if(current_process->nchildren > 1){
        fprintf(stdout, "─┬─");
        print(current_process->cpid[0], prefix);
    }
    for(unsigned int i=1; i<current_process->nchildren ;++i){
        prefix[prefix_size-3] = '\0';
        fprintf(stdout, "%s", prefix);
        prefix[prefix_size-3] = ' ';
        if(i+1 != current_process->nchildren){
            fprintf(stdout, " ├─");
        }else{
            fprintf(stdout, " └─");
        }
        print(current_process->cpid[i], prefix);
    }

    // ------------------ unset prefix ------------------
    prefix[prefix_size-name_size-3] = '\0';
    prefix_size -= name_size + 3;


    return;
}

int main(){
    // read pid_max 
    const int PID_MAX = getPID_MAX();

    // alloc space
    process_table = (struct Process**)malloc(sizeof(struct Process*) * (PID_MAX+1));
    for(int i=0;i<=PID_MAX;++i){
        process_table[i] = NULL;
    }

    // set Process table
    setProcess_table(process_table, PID_MAX);
    
    // Print process tree
    char prefix[NMAX_PREFIX];
    prefix[0] = '\0';
    print(1, prefix);

    //  Recycle Space
    for(int i=0;i<=PID_MAX;++i){
        if(process_table[i] != NULL && process_table[i]->name != NULL){
            free(process_table[i]->name);
            free(process_table[i]);
        }else if(process_table[i] != NULL){
            free(process_table[i]);
        }
    }
    free(process_table);

    return 0;
}