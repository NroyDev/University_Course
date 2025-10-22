/*
 * is_background.c :  check for & at end
 */

#include <stdio.h>
#include "shell.h"

int is_background(char ** myArgv) {

  	if (*myArgv == NULL)
    	return 0;

  	/* Look for "&" in myArgv, and process it.
  	 *
	 *	- Return TRUE if found.
	 *	- Return FALSE if not found.
	 *
	 * Fill in code.
	 */
	int idx = 0;
	while(myArgv[idx] != NULL){
		++idx;
	}
	if(idx-1>=0 && strcmp(myArgv[idx-1],"&")==0){
		free(myArgv[idx-1]);
		myArgv[idx-1] = NULL;
		return TRUE;
	}

	 return FALSE;
}