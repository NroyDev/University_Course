/*
 * lookup1 : straight linear search through a local file
 * 	         of fixed length records. The file name is passed
 *	         as resource.
 */
#include <string.h>
#include <stdlib.h>
#include "dict.h"

int lookup(Dictrec * sought, const char * resource) {
	Dictrec dr;
	static FILE * in;
	static int first_time = 1;

	if (first_time) { 
		first_time = 0;
		/* open up the file
		 *
		 * Fill in code. */
		if ((in =fopen(resource,"r")) == NULL){DIE(resource);}
	}

	/* read from top of file, looking for match
	 *
	 * Fill in code. */
	rewind(in);
	// remove \n from sought->word
	int sought_word_size = strlen(sought->word);
	if(sought_word_size-1>=0 && sought->word[sought_word_size-1]=='\n'){
		sought->word[sought_word_size-1] = '\0';
		--sought_word_size;
	}
	while(!feof(in)){
		for(int i=0;i<sizeof(Dictrec);++i){
			int c = getc(in);
			((unsigned char*)&dr)[i] = (unsigned char)c;
		}
		if(strcmp(dr.word, sought->word) == 0){
			break;
		}
	}
	while(strcmp(dr.word, sought->word) == 0) {
		/* Fill in code. */
		for(int i=0;i<TEXT;++i){
			sought->text[i] = dr.text[i];
		}
		return FOUND;
	}

	return NOTFOUND;
}
