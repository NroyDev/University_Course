#include<stdio.h>
#include<stdlib.h>    // atoi()
#include<unistd.h>
#include<sys/wait.h>  // wait()

int main(int argc,char* argv[]){
  // 用法 ./hw1-1 [想執行fork()的次數]
  // 備註： 如果沒有輸入想fork()的次數 則預設為3


  if(argc>1){
    int n = atoi(argv[1]);
    printf("As you can see,there are %d (2^%d-1) threads that has been created. \n",(1<<n-1),n);
    printf("So,The program has %d (2^%d) Threads in total\n",(1<<n),n);
    for(int i=0;i<n;++i){
      fork();
    }
    printf("CURRENT THREAD PID = %d / Parent PID = %d\n",getpid(),getppid());
  }else{
    printf("As you can see,there are 7 (2^3-1) threads that has been created. \n");
    printf("So,The program has 8 (2^3) Threads in total\n");
    fork();
    fork();
    fork();
    printf("CURRENT THREAD PID = %d / Parent PID = %d\n",getpid(),getppid());
  }
  int status;
  while(wait(&status)>0){
  }
  return 0;
}
