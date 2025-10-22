/*
 * redirect_in.c  :  check for <
 */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "shell.h"
#define STD_OUTPUT 1
#define STD_INPUT  0

/*
 * Look for "<" in myArgv, then redirect input to the file.
 * Returns 0 on success, sets errno and returns -1 on error.
 */
int redirect_in(char ** myArgv) {
  	int i = 0;
  	int fd;

  	/* search forward for <
  	 *
	 * Fill in code. */
	for(i = 0; myArgv[i] != NULL; ++i){
		if(strcmp(myArgv[i], "<") == 0){
			break;
		}
	}

  	if (myArgv[i]) {	/* found "<" in vector. */

    	/* 1) Open file.
     	 * 2) Redirect stdin to use file for input.
   		 * 3) Cleanup / close unneeded file descriptors.
   		 * 4) Remove the "<" and the filename from myArgv.
		 *
   		 * Fill in code. */

		// 0) check has filepath
		if(myArgv[i+1] == NULL){
			// const char* errorMsg = "Error - 在 < 後面缺少檔案\n";
			// const int size = strlen(errorMsg);
			// if(write(STDERR_FILENO, errorMsg, size) != size){
			// 	return -1;
			// }
			return -1;
		}

		// 1) Open file.
		if((fd=open(myArgv[i+1], O_RDONLY)) == -1){
			return -1;
		}

		// 2) Redirect stdin to use file for input.
		if(close(STD_INPUT) == -1){
			return -1;
		}
		if(dup2(fd, STD_INPUT) == -1){
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
