/*
 * convert.c : take a file in the form 
 *  word1
 *  multiline definition of word1
 *  stretching over several lines, 
 * followed by a blank line
 * word2....etc
 * convert into a file of fixed-length records
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "dict.h"
#define BIGLINE 512

int main(int argc, char **argv) {
	FILE *in;
	FILE *out;        /* defaults */
	char line[BIGLINE];
	static Dictrec dr, blank;
	
	/* If args are supplied, argv[1] is for input, argv[2] for output */
	if (argc==3) {
		if ((in =fopen(argv[1],"r")) == NULL){DIE(argv[1]);}
		if ((out =fopen(argv[2],"w")) == NULL){DIE(argv[2]);}	
	}
	else{
		printf("Usage: convert [input file] [output file].\n");
		return -1;
	}

	/* Main reading loop : read word first, then definition into dr */

	/* Loop through the whole file. */
	while (!feof(in)) {
		
		/* Create and fill in a new blank record.
		 * First get a word and put it in the word field, then get the definition
		 * and put it in the text field at the right offset.  Pad the unused chars
		 * in both fields with nulls.
		 */
		line[0] = '\0';
		dr = blank;

		/* Read word and put in record.  Truncate at the end of the "word" field.
		 *
		 * Fill in code. */
		fgets(line, BIGLINE, in);
		if(line[0] == '\0'){
			break;
		}
		int word_size = strlen(line);
		if(word_size-1>=0 && line[word_size-1]=='\n'){
			line[word_size-1] = '\0';
			--word_size;
		}
		strncpy(dr.word, line, WORD-1);	// turncate & pad unused char with null bye

		/* Read definition, line by line, and put in record.
		 *
		 * Fill in code. */
		int text_size = 0;
		while(fgets(line, BIGLINE, in) != NULL && strcmp("\n",line) != 0){
			int line_size = strlen(line);
			if(line_size-1>=0 && line[line_size-1]=='\n'){
				line[line_size-1] = '\0';
				--line_size;
			}
			char* next_start = strncpy(dr.text+text_size,line, TEXT-1-text_size);	// turncate & pad unused char with null bye
			text_size = next_start - dr.text;
		}

		/* Write record out to file.
		 *
		 * Fill in code. */
		for(int i=0;i<sizeof(Dictrec);++i){
			putc(((unsigned char*)&dr)[i], out);
		}
	}

	fclose(in);
	fclose(out);
	return 0;
}
