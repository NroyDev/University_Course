/* 
 * pipe_command.c  :  deal with pipes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "shell.h"

#define STD_OUTPUT 1
#define STD_INPUT  0

void pipe_and_exec(char **myArgv) {
  	int pipe_argv_index = pipe_present(myArgv);
  	int pipefds[2];
	char **left_argv;
	char **right_argv;

  	switch (pipe_argv_index) {

    	case -1:	/* Pipe at beginning or at end of argv;  See pipe_present(). */
      		fputs ("Missing command next to pipe in commandline.\n", stderr);
      		errno = EINVAL;	/* Note this is NOT shell exit. */
      		break;

    	case 0:	/* No pipe found in argv array or at end of argv array.
			See pipe_present().  Exec with whole given argv array. */
			if(execvp(myArgv[0], myArgv) == -1){
				// should not be executed below
				perror("Error - left command execvp");
				exit(errno);
			}
      		break;

    	default:	/* Pipe in the middle of argv array.  See pipe_present(). */

      		/* Split arg vector into two where the pipe symbol was found.
       		 * Terminate first half of vector.
			 *
       		 * Fill in code. */
			left_argv = myArgv;
			free(myArgv[pipe_argv_index]);
			myArgv[pipe_argv_index] = NULL;
			right_argv = myArgv+pipe_argv_index+1;	// 不用擔心超出範圍 pipe_present() 保證超出範圍會回傳-1

      		/* Create a pipe to bridge the left and right halves of the vector. 
			 *
			 * Fill in code. */
			if(pipe(pipefds) == -1){
				perror("Error - pipe");
				exit(errno);
			}

      		/* Create a new process for the right side of the pipe.
       		 * (The left side is the running "parent".)
       		 *
			 * Fill in code to replace the underline. */
      		switch(fork()){

        		case -1 :
	  				break;

        		/* Talking parent.  Remember this is a child forked from shell. */
        		default :

	  				/* - Redirect output of "parent" through the pipe.
	  				 * - Don't need read side of pipe open.  Write side dup'ed to stdout.
	 	 			 * - Exec the left command.
					 *
					 * Fill in code. */
					if(close(pipefds[0]) == -1|| close(STD_OUTPUT) == -1){	// close pipe read, close stdout
						perror("Error - close");
						exit(errno);
					}
					if(dup2(pipefds[1], STD_OUTPUT) == -1){
						perror("Error - dup2");
						exit(errno);
					}
					if(close(pipefds[1] == -1)){
						perror("Error - close");
						exit(errno);
					}

					if(execvp(left_argv[0], left_argv) == -1){
						// should not be executed below
						perror("Error - left command execvp");
						exit(errno);
					}

	  				break;

        		/* Listening child. */
        		case 0 :

	  				/* - Redirect input of "child" through pipe.
					  * - Don't need write side of pipe. Read side dup'ed to stdin.
				  	 * - Exec command on right side of pipe and recursively deal with other pipes
					 *
					 * Fill in code. */
					if(close(pipefds[1]) == -1|| close(STD_INPUT) == -1){	// close pipe write, close stdin
						perror("Error - close");
						exit(errno);
					}
					if(dup2(pipefds[0], STD_INPUT) == -1){
						perror("Error - dup2");
						exit(errno);
					}
					if(close(pipefds[0])  == -1){
						perror("Error - close");
						exit(errno);
					}
					 
          			pipe_and_exec(&myArgv[pipe_argv_index+1]);
			}
	}
	perror("Couldn't fork or exec child process");
  	exit(errno);
}
