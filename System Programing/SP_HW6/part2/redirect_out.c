/*
 * redirect_out.c   :   check for >
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "shell.h"
#define STD_OUTPUT 1
#define STD_INPUT  0

/*
 * Look for ">" in myArgv, then redirect output to the file.
 * Returns 0 on success, sets errno and returns -1 on error.
 */
int redirect_out(char ** myArgv) {
	int i = 0;
  	int fd;

  	/* search forward for >
  	 * Fill in code. */
	for(i = 0; myArgv[i] != NULL; ++i){
		if(strcmp(myArgv[i], ">") == 0){
			break;
		}
	}

  	if (myArgv[i]) {	/* found ">" in vector. */

    	/* 1) Open file.
    	 * 2) Redirect to use it for output.
    	 * 3) Cleanup / close unneeded file descriptors.
    	 * 4) Remove the ">" and the filename from myArgv.
		 *
    	 * Fill in code. */

		// 0) check has filepath
		if(myArgv[i+1] == NULL){
			return -1;
		}

		// 1) Open file.
		if((fd=open(myArgv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1){
			return -1;
		}

		// 2) Redirect stdin to use file for output.
		if(close(STD_OUTPUT) == -1){
			return -1;
		}
		if(dup2(fd, STD_OUTPUT) == -1){
			return -1;
		}

   		// 3) Cleanup / close unneeded file descriptors.
		if(close(fd) == -1){
			return -1;
		}

		// 4) Remove the "<" and the filename from myArgv.
		free(myArgv[i]);
		free(myArgv[i+1]);
		while(myArgv[i+2] != NULL){		// 把後面的前移
			myArgv[i] = myArgv[i+2];
			i += 1;
		}
		myArgv[i] = NULL;
		myArgv[i+1] = NULL;
  	}
  	return 0;
}
