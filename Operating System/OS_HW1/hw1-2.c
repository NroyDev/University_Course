#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>  // wait()
int main(){
  pid_t pid = vfork();
  if(pid<0){
    printf("vfork() failed!");
    exit(-1);
  }else if(pid==0){ //child
    close(0); //close stdin
    printf("Child:  Now, child thread closed file descriptor 0 (stdin).\n");
    exit(0);
  }else{  //parent
    int temp;
    wait(&temp);    // ensure that child has closed it file descriptor 0
    printf("Parent: Input a int: ");
    scanf("%d",&temp);
    printf("Parent: Yes,it still working, you just entered: %d\n",temp);
  }

  return 0;
}
