/**
 * This is the symtab file for Pamx
 * These functions are used to manage the parser symbol table - the table
 * 	used to house the variables and identifiers in the language
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

//int levels;
symbol * level_placements[1024];
symbol symtab[NHASH];

/**
 * symhash - the hash function for the symbol hash table
 * sym  the name of the symbol that the hash is being calculated for
 * return hash - returns an unsigned int of the hash value (what placement the
 * 		 symbol is located in the symbol table)
 */
static unsigned symhash(char * sym) {
	unsigned int hash = 0;
	unsigned c;

	for(int i = 0; i < strlen(sym); i++) {
		c = (int)sym[i];
		hash = hash * 9 ^ c;
	}	

	return hash;
}

/**
 * lookup - looks up a symbol in the symbol table by the symbol's
 * 	    name and type
 * sym  the name of the symbol being looked up
 * type  the type of the symbol being looked up (file, user, label, or level)
 */
symbol * lookup(char * sym, enum type type) {
	symbol * sp = &symtab[symhash(sym)%NHASH];
	int scount = NHASH;
	while(--scount >= 0) {
		// Found symbol in table
		if(sp->name && !strcasecmp(sp->name, sym)) {
			sp->new_sym = 0;
			return sp;
		}
		// New symbol
		if(!sp->name) {
			sp->new_sym = 1;
			sp->name = strdup(sym);
			sp->ref_list = malloc(sizeof(utype *));
			sp->type = type;
			return sp;
		}

		if(++sp >= symtab+NHASH) sp = symtab;
	}

	fputs("symbol table overflow\n", stderr);
	abort(); // Table full
}


/**
 * add_level - add symbol of the type level to the symbol table
 * lineno  the line number that the level symbol is on in the input file
 * word  the name of the symbol being added
 */
void add_level(int lineno, int placement, char * word) {
	levelRef * lr;
	symbol * sp = lookup(word, LEVEL);

	// reference found on same line. do not add to symtab
	if(sp->ref_list[0].level &&
	   sp->ref_list[0].level->lineno == lineno) return; 

	lr = malloc(sizeof(levelRef *));
	
	if(!lr) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	lr->next = sp->ref_list;
	lr->lineno = lineno;
	lr->placement = placement;
	sp->ref_list->level = lr;
}

/**
 * add_label - add symbol of the type label to the symbol table
 * lineno  the line number that the label symbol is on in the input file
 * word  the name of the symbol being added
 */
void add_label(int lineno, char * word) {
	labelRef * lr;
	symbol * sp = lookup(word, LABEL);
	
	// reference found on same line
	if(sp->ref_list[0].label &&
	   sp->ref_list[0].label->lineno == lineno) return; 
	
	lr = malloc(sizeof(labelRef *));
	
	if(!lr) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	lr->next = sp->ref_list;
	lr->lineno = lineno;
	sp->ref_list->label = lr;
}

/**
 * add_user - add symbol of type user to the symbol table
 * lineno  the line number that the user symbol is on in the input file
 * word  the name of the symbol being added
 */
void add_user(int lineno, char * word) {
	userRef * ur;
	symbol * sp = lookup(word, USER_NAME);
	
	// reference found on same line
	if(sp->ref_list &&
	   sp->ref_list->user->lineno == lineno) return; 
	ur = malloc(sizeof(struct userRef *));
	
	if(!ur) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	ur->next = sp->ref_list;
	ur->lineno = lineno;
	sp->ref_list->user = ur;
}

/**
 * add_file - add symbol of type file to the symbol table
 * lineno  the line number that the file symbol is on in the input file
 * word  the name of the symbol being added
 */
void add_file(int lineno, char * word) {
	fileRef * fr;
	symbol * sp = lookup(word, FILE_NAME);
	
	// reference found on same line
	if(sp->ref_list &&
	   sp->ref_list->file->lineno == lineno) return; 
	
	fr = malloc(sizeof(fileRef *));
	
	if(!fr) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	fr->next = sp->ref_list;
	fr->lineno = lineno;
	sp->ref_list->file = fr;
}

/**
 * format_level_data - return level data similar to selinux. <level_name>:<level_placement>
 * sym  the symbol of type level
 * returns string representing data for the level symbol delimited by :
 */
char * format_level_data(symbol * sym) {
	if(sym->ref_list[0].level == NULL) {
		return "";
	}
	char * levelData = malloc(200);
	sprintf(levelData, "%s:%d", strdup(sym->name), sym->ref_list[0].level->placement);
	return levelData;
}

