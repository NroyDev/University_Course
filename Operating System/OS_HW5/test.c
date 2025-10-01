#include<stdio.h>
#include<stdlib.h>
#define N 2000
int main(){
	for(int i=0;i<N;++i){
		printf("Round %d: ",i+1);
		int ret = system("./hw5");
		
		if(ret==-1){
			perror("system failed");
			printf("system failed at %d\n",i);
			return 1;
		}
		printf("\n");
	}
	
	return 0;
}
