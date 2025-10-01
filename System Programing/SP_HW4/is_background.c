/*
 * is_background.c :  check for & at end
 */


#include <stdio.h>
#include "shell.h"
#include <string.h>

int is_background(char** myArgv) {
	if (*myArgv == NULL){
		return 0;
	}

	/* Look for "&" in myArgv, and process it.
	 *
	 *	- Return TRUE if found.
	 *	- Return FALSE if not found.
	 *
	 * Fill in code.
	*/
	int argc = 0;
	while(myArgv[argc] != NULL){
		++argc;
	}
	if(strcmp(myArgv[argc-1], "&") == 0){
		return 1;	// true
	}else{
		return 0;	// false
	}
}
