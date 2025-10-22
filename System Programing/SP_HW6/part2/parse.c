/*
 * parse.c : use whitespace to tokenise a line
 * Initialise a vector big enough
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"

#include <errno.h>

/* Parse a commandline string into an argv array. */
char ** parse(char *line) {

  	static char delim[] = " \t\n"; /* SPACE or TAB or NL */
  	int count = 0;
  	char * token;
  	char **newArgv;

	// ------------------------------------------ 提前離開的狀況 ------------------------------------------
  	/* Nothing entered. */
  	if (line == NULL || strcmp(line,"\n")==0) {
    	return NULL;
  	}

  	/* Init strtok with commandline, then get first token.
     * Return NULL if no tokens in line.
	 *
	 * Fill in code.
     */
	if((token = strtok(line, delim)) == NULL){
		return NULL;
	}


	// ------------------------------------------ 處理第一個token ------------------------------------------
  	/* Create array with room for first token.
  	 *
	 * Fill in code.
	 */
	// Resize Array(0=>1)
	count = 1;		// array size
	if((newArgv = (char**)malloc(sizeof(char*)*count)) == NULL){
		fprintf(stderr, "Error: malloc - %s\n", strerror(errno));
		exit(errno);
	}
	// malloc install
	const int token_size = strlen(token)+1;         // include \0
	if((newArgv[count-1] = (char*)malloc(sizeof(char)* token_size)) == NULL){
		fprintf(stderr, "Error: malloc - %s\n", strerror(errno));
		exit(errno);
	}
	strcpy(newArgv[count-1], token);
	fprintf(stdout, "[%d] : %s\n", count-1, token);

	// ------------------------------------------ 處理後續的token ------------------------------------------
  	/* While there are more tokens...
	 *
	 *  - Get next token.
	 *	- Resize array.
	 *  - Give token its own memory, then install it.
	 * 
  	 * Fill in code.
	 */
	while((token = strtok(NULL, delim)) != NULL){
		// Resize Array
		++count;
		if((newArgv = (char**)realloc(newArgv, sizeof(char*)*count)) == NULL){
			fprintf(stderr, "Error: realloc - %s\n",strerror(errno));
			exit(errno);
		}

		// malloc install
		const int token_size = strlen(token)+1;		// include \0
		if((newArgv[count-1] = (char*)malloc(sizeof(char)*token_size)) == NULL){
			fprintf(stderr, "Error: malloc - %s\n", strerror(errno));
			exit(errno);
		}
		strcpy(newArgv[count-1], token);
		fprintf(stdout, "[%d] : %s\n", count-1, token);
	}

	// ------------------------------------------ 在最後面放NULL ------------------------------------------
  	/* Null terminate the array and return it.
	 *
  	 * Fill in code.
	 */
	++count;
	if((newArgv = (char**)realloc(newArgv, sizeof(char*)*count)) == NULL){
		fprintf(stderr, "Error: realloc - %s\n", strerror(errno));
		exit(errno);
	}
	newArgv[count-1] = NULL;

  	return newArgv;
}


/*
 * Free memory associated with argv array passed in.
 * Argv array is assumed created with parse() above.
 */
void free_argv(char **oldArgv) {

	int i = 0;

	/* Free each string hanging off the array.
	 * Free the oldArgv array itself.
	 *
	 * Fill in code.
	 */
	while(oldArgv[i] != NULL){
		free(oldArgv[i]);
		++i;
	}
	free(oldArgv);

	return;
}
