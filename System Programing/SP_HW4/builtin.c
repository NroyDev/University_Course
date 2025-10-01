/*
 * builtin.c : check for shell built-in commands
 * structure of file is
 * 1. definition of builtin functions
 * 2. lookup-table
 * 3. definition of is_builtin and do_builtin
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"

#include <string.h>
#include <errno.h>

/****************************************************************************/
/* builtin function definitions                                             */
/****************************************************************************/

/* "echo" command.  Does not print final <CR> if "-n" encountered. */
static void bi_echo(char **argv) {
	/* Fill in code. */
	int argc = 0;
	while(argv[argc] != NULL){
		++argc;
	}
	
	// XXX:
	// REVIEW: 實作依照最新 PDF，但與先前註解邏輯不一致，需確認哪個版本為準。
	// XXX:
	// parse 參數
	int is_specified = 0;
	int specified_arg = 0;
	if(argc > 1 && strcmp(argv[1],"-n") == 0){
		is_specified = 1;
		if(argc > 2){
			char* endptr = NULL;
			errno = 0;
			specified_arg = strtol(argv[2], &endptr, 10);
			if(errno != 0){
				fprintf(stderr, "Error: strtol - %s\n", strerror(errno));
				exit(errno);
			}else if(endptr == argv[2]){
				fprintf(stderr, "Error: strtol - %s\n", "未能找到可轉換的數字(invalid input)\n");
				exit(-1);
			}else if(*endptr != '\0'){
				fprintf(stderr, "Warning: strtol - 數字後帶有一些非數字部份 %s\n", endptr);
			}else if(specified_arg <0){
				fprintf(stderr, "Error: 不可指定負數 %d\n", specified_arg);
				exit(-1);
			}else if(specified_arg >= argc){
				fprintf(stderr, "Error: %d 超出範圍\n", specified_arg);
				exit(-1);
			}
		}else{
			fprintf(stderr, "Error: 指定了-n 但未給予後續指定參數\n");
			exit(-1);
		}
	}

	// 輸出到stdin上
	if(is_specified){
		fprintf(stdout, "%s\n", argv[specified_arg+2]);
	}else{
		for(int i=1; i<argc-1; ++i){
			fprintf(stdout, "%s ", argv[i]);
		}
		fprintf(stdout, "%s\n", argv[argc-1]);
	}
}
/* Fill in code. */
// Terminate Shell
static void exit_shell(char** argv){
	free_argv(argv);
	exit(0);
}



/****************************************************************************/
/* lookup table                                                             */
/****************************************************************************/

static struct cmd {
	char * keyword;				/* When this field is argv[0] ... */
	void (* do_it)(char **);	/* ... this function is executed. */
} inbuilts[] = {
	
	/* Fill in code. */
	{"echo", bi_echo},		/* When "echo" is typed, bi_echo() executes.  */
	{"exit", exit_shell},
	{"quit", exit_shell},
	{"logout", exit_shell},
	{"bye", exit_shell},
	{NULL, NULL}				/* NULL terminated. */
};




/****************************************************************************/
/* is_builtin and do_builtin                                                */
/****************************************************************************/

static struct cmd * this; 		/* close coupling between is_builtin & do_builtin */

/* Check to see if command is in the inbuilts table above.
Hold handle to it if it is. */
int is_builtin(char *cmd) {
  	struct cmd *tableCommand;
	
  	for (tableCommand = inbuilts ; tableCommand->keyword != NULL; tableCommand++)
    	if (strcmp(tableCommand->keyword,cmd) == 0) {
			this = tableCommand;
			return 1;
		}
  	return 0;
}


/* Execute the function corresponding to the builtin cmd found by is_builtin. */
int do_builtin(char **argv) {	
	// FIXME: 回傳值？？？ Return 啥？
  	this->do_it(argv);
}
