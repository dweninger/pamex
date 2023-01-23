#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

int levels;
symbol * levelplacements[1024];
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
			sp->newSym = 0;
			return sp;
		}
		// New symbol
		if(!sp->name) {
			sp->newSym = 1;
			sp->name = strdup(sym);
			sp->reflist = malloc(sizeof(utype *));
			sp->type = type;
			return sp;
		}

		if(++sp >= symtab+NHASH) sp = symtab;
	}

	fputs("symbol table overflow\n", stderr);
	abort(); // Table full
}


/**
 * addlevel - add symbol of the type level to the symbol table
 * lineno  the line number that the level symbol is on in the input file
 * word  the name of the symbol being added
 */
void addlevel(int lineno, int placement, char * word) {
	levelRef * lr;
	symbol * sp = lookup(word, LEVEL);

	// reference found on same line. do not add to symtab
	if(sp->reflist[0].level &&
	   sp->reflist[0].level->lineno == lineno) return; 

	lr = malloc(sizeof(levelRef *));
	
	if(!lr) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	lr->next = sp->reflist;
	lr->lineno = lineno;
	lr->placement = placement;
	sp->reflist->level = lr;
}

/**
 * addlabel - add symbol of the type label to the symbol table
 * lineno  the line number that the label symbol is on in the input file
 * word  the name of the symbol being added
 */
void addlabel(int lineno, char * word) {
	labelRef * lr;
	symbol * sp = lookup(word, LABEL);
	
	// reference found on same line
	if(sp->reflist[0].label &&
	   sp->reflist[0].label->lineno == lineno) return; 
	
	lr = malloc(sizeof(labelRef *));
	
	if(!lr) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	lr->next = sp->reflist;
	lr->lineno = lineno;
	sp->reflist->label = lr;
}

/**
 * adduser - add symbol of type user to the symbol table
 * lineno  the line number that the user symbol is on in the input file
 * word  the name of the symbol being added
 */
void adduser(int lineno, char * word) {
	userRef * ur;
	symbol * sp = lookup(word, USER_NAME);
	
	// reference found on same line
	if(sp->reflist &&
	   sp->reflist->user->lineno == lineno) return; 
	ur = malloc(sizeof(struct userRef *));
	
	if(!ur) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	ur->next = sp->reflist;
	ur->lineno = lineno;
	sp->reflist->user = ur;
}

/**
 * addfile - add symbol of type file to the symbol table
 * lineno  the line number that the file symbol is on in the input file
 * word  the name of the symbol being added
 */
void addfile(int lineno, char * word) {
	fileRef * fr;
	symbol * sp = lookup(word, FILE_NAME);
	
	// reference found on same line
	if(sp->reflist &&
	   sp->reflist->file->lineno == lineno) return; 
	
	fr = malloc(sizeof(fileRef *));
	
	if(!fr) { 
		fputs("out of space\n", stderr); 
		abort(); 
	}
	
	fr->next = sp->reflist;
	fr->lineno = lineno;
	sp->reflist->file = fr;
}

/**
 * leveltojson - return json data from the level symbol passed in
 * sym  the symbol of type level
 * returns string representing JSON data for the level symbol
 */
char * leveltojson(symbol * sym) {
	if(sym->reflist[0].level == NULL) {
		return "";
	}
	char * jsondata = malloc(sizeof(100));
	sprintf(jsondata, "{\"%s\":\"%s\",\"%s\":%d}", "name", strdup(sym->name), "placement", sym->reflist[0].level->placement);
	return jsondata;
}

/**
 * labeltojson - return json data from the label symbol passed in
 * sym  the symbol of type label
 * returns string representing JSON data for the label symbol
 */
char * labeltojson(symbol * sym) {
	if(sym->reflist[0].label == NULL) {
		return "";
	}
	char * jsondata = malloc(sizeof(100));
	snprintf(jsondata, sizeof(jsondata), "{\"%s\":\"%s\"}", "name", strdup(sym->name));
	return jsondata;
}
