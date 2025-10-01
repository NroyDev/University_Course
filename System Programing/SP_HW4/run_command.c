/*
* run_command.c :    do the fork, exec stuff, call other functions
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "shell.h"

#include <string.h>
#include <unistd.h>

void run_command(char** myArgv){
	/* Create a new child process.
	* Fill in code.*/
	pid_t pid = fork();
	int run_background = is_background(myArgv);
	int argc = 0;
	while(myArgv[argc] != NULL){
		++argc;
	}
	const char* last_argv = myArgv[argc-1];
	if(run_background){	// 要清除最後一個argv &
		myArgv[argc-1] = NULL;
	}


	switch(pid){
		case -1:{	/* Error. */
			perror("fork");
			exit(errno);
		}
		case 0:{	/* Child. */
			/* Run command in child process.
			* Fill in code.*/
			execvp(myArgv[0], myArgv);
			/* Handle error return from exec 如果會執行到這邊的code 代表execvp失敗了 */
			printf("Error: execvp - %s\n", strerror(errno));
			exit(errno);
		}
		default:{	/* Parent. */
			/* Wait for child to terminate.
			* Fill in code.*/
			pid_t wpid;
			int wstatus;
			int options = (run_background) ? WNOHANG : 0;
			if((wpid= waitpid(pid, &wstatus, options)) < 0){
				fprintf(stderr, "Error: waitpid - %s\n", strerror(errno));
				exit(errno);
			}

			/* Optional: display exit status.  (See wstat(5).)
			* Fill in code.*/
			if(!run_background){
				if(WIFEXITED(wstatus)){
					fprintf(stdout, "Process %s(%d) exit, status=%d\n", myArgv[0], wpid, WEXITSTATUS(wstatus));
				}else if(WIFSIGNALED(wstatus)){
					fprintf(stdout, "Process %s(%d) 被signal(%s/%d) 幹掉了\n", myArgv[0], wpid, strsignal(WTERMSIG(wstatus)), WTERMSIG(wstatus));
				}else{
					fprintf(stdout, "Process %s(%d) 異常退出\n", myArgv[0], wpid);
				}
			}
		}
	}


	if(run_background){	// 把最後一個 argv &還回來 以便待會系統回收空間
		myArgv[argc-1] = (char*)last_argv;
	}
	return;
}
