/*
 * builtin.c : check for shell built-in commands
 * structure of file is
 * 1. definition of builtin functions
 * 2. lookup-table
 * 3. definition of is_builtin and do_builtin
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/utsname.h>
#include "shell.h"

/****************************************************************************/
/* builtin function definitions                                             */
/****************************************************************************/
static void bi_builtin(char ** argv);	/* "builtin" command tells whether a command is builtin or not. */
static void bi_cd(char **argv) ;		/* "cd" command. */
static void bi_echo(char **argv);		/* "echo" command.  Does not print final <CR> if "-n" encountered. */
static void bi_hostname(char ** argv);	/* "hostname" command. */
static void bi_id(char ** argv);		/* "id" command shows user and group of this process. */
static void bi_pwd(char ** argv);		/* "pwd" command. */
static void bi_quit(char **argv);		/* quit/exit/logout/bye command. */




/****************************************************************************/
/* lookup table                                                             */
/****************************************************************************/

static struct cmd {
  	char * keyword;					/* When this field is argv[0] ... */
  	void (* do_it)(char **);		/* ... this function is executed. */
} inbuilts[] = {
  	{ "builtin",    bi_builtin },   /* List of (argv[0], function) pairs. */

    /* Fill in code. */
    { "echo",       bi_echo },
    { "quit",       bi_quit },
    { "exit",       bi_quit },
    { "bye",        bi_quit },
    { "logout",     bi_quit },
    { "cd",         bi_cd },
    { "pwd",        bi_pwd },
    { "id",         bi_id },
    { "hostname",   bi_hostname },
    {  NULL,        NULL }          /* NULL terminated. */
};


static void bi_builtin(char ** argv) {
	/* Fill in code. */
	// XXX: 去確認一下 "The builtin command lists the functions built into your shell" 是什麼意思？ 跟題目圖片敘述有差異？ 這邊依照圖片敘述實作
	if(argv == NULL || argv[0] == NULL || argv[1] == NULL || argv[2] != NULL){
		fprintf(stderr, "Error - builtin with wrong argv\n");
		exit(-1);
	}
	if(is_builtin(argv[1])){
		fprintf(stdout, "%s is a builtin feature\n", argv[1]);
	}else{
		fprintf(stdout, "%s is NOT a builtin feature\n", argv[1]);
	}
}

static void bi_cd(char **argv) {
	/* Fill in code. */
	if(argv == NULL || argv[0] == NULL || argv[1] == NULL || argv[2] != NULL){
		fprintf(stderr, "Error - cd with wrong argv\n");
		exit(-1);
	}
	if(chdir(argv[1]) == -1){
		perror("Error - chdir");
		exit(errno);
	}
}

static void bi_echo(char **argv) {
	/* Fill in code. */
	if(argv == NULL || argv[0] == NULL){
		fprintf(stderr, "Error - echo with wrong argv\n");
		exit(-1);
	}
	if(argv[1] != NULL){
		fprintf(stdout, "%s", argv[1]);
	}
	int i = 2;
	while(argv[i] != NULL){
		fprintf(stdout, " %s", argv[i]);
		++i;
	}
	fprintf(stdout, "\n");
}

static void bi_hostname(char ** argv) {
	/* Fill in code. */
	if(argv == NULL || argv[0] == NULL){
		fprintf(stderr, "Error - hostname with wrong argv\n");
		exit(-1);
	}
	struct utsname uts;
    if(uname(&uts) == -1){
        perror("Error - uname:");
        exit(errno);
    }
    fprintf(stdout, "hostname: %s\n", uts.nodename);
}

static void bi_id(char ** argv) {
 	/* Fill in code. */
	if(argv == NULL || argv[0] == NULL){
		fprintf(stderr, "Error - id with wrong argv\n");
		exit(-1);
	}
	struct passwd *pw;
	if((pw = getpwuid(getuid())) == NULL){
		perror("Error - getpwuid");
		exit(errno);
	}
	char* username = (char*)malloc(sizeof(char)*(strlen(pw->pw_name)+1));
	strcpy(username, pw->pw_name);
	struct group *gr;
	if((gr = getgrgid(getgid())) == NULL){
		perror("Error - getgrgid");
		exit(errno);
	}
	char* groupname = (char*)malloc(sizeof(char)*(strlen(gr->gr_name)+1));
	strcpy(groupname, gr->gr_name);

	fprintf(stdout, "UserID = %d(%s), Group ID = %d(%s)\n", getuid(), username, getgid(), groupname);

	free(username);
	free(groupname);
}

static void bi_pwd(char ** argv) {
	/* Fill in code. */
	if(argv == NULL || argv[0] == NULL){
		fprintf(stderr, "Error - pwd with wrong argv\n");
		exit(-1);
	}
    // get max path
    errno = 0;
    long pathmaxlen = pathconf(".", _PC_PATH_MAX);
    if(pathmaxlen == -1 && errno!=0){
        perror("Error - pathconf");
        exit(errno);
    }else if(pathmaxlen == -1){  // limit is indeterminate
        fprintf(stderr, "Warning - pathconf _PC_PATH_MAX limit is indeterminate\n");
        pathmaxlen = 4096;
    }
    const long PATH_MAX_LEN = pathmaxlen;

    char *buf = (char*)malloc(sizeof(char)*(PATH_MAX_LEN+1));
    if(buf == NULL){
        perror("Error - malloc");
        exit(errno);
    }
    if(getcwd(buf, PATH_MAX_LEN+1) == NULL){
        perror("Error - getcwd");
        exit(errno);
    }
    fprintf(stdout, "%s\n", buf);

    free(buf);
}

static void bi_quit(char **argv) {
	exit(0);
}


/****************************************************************************/
/* is_builtin and do_builtin                                                */
/****************************************************************************/

static struct cmd * this; /* close coupling between is_builtin & do_builtin */

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
	this->do_it(argv);
}
