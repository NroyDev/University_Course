#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/utsname.h>

int main(){
    struct utsname uts;
    if(uname(&uts) == -1){
        perror("Error - uname:");
        exit(errno);
    }
    long hostid = gethostid();
    if(hostid == -1){
        perror("Error - gethostid:");
        exit(errno);
    }

    fprintf(stdout, "hostname: %s\n", uts.nodename);
    fprintf(stdout, "%s\n", uts.release);
    fprintf(stdout, "hostid: %ld\n", hostid);


    return 0;
}