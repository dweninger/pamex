#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

int levels;
symbol * levelplacements[1024];
symbol symtab[NHASH];

static unsigned symhash(char * sym) {
	printf("symhash %s, %d\n", sym, strlen(sym));
	unsigned int hash = 0;
	unsigned c;

	for(int i = 0; i < strlen(sym); i++) {
		c = (int)sym[i];
		hash = hash * 9 ^ c;
	}	

	//while(c = *sym++) {
	//	printf("symhash while: %c\n", c); 
	//	hash = hash*9 ^ c;
	//}
	printf("HASH: %u\n", hash);
	return hash;
}

symbol * lookup(char * sym, enum type type) {
	printf("lookup\n");
	symbol * sp = &symtab[symhash(sym)%NHASH];
	int scount = NHASH;
	printf("Lookup\n");
	while(--scount >= 0) {
		// Found symbol in table
		if(sp->name && !strcasecmp(sp->name, sym)) {
			sp->newSym = 0;
		       	printf("SP NAME1: %s\n", sp->name);
			return sp;
		}
		// New symbol
		if(!sp->name) {
			sp->newSym = 1;
			sp->name = strdup(sym);
			sp->reflist = malloc(sizeof(utype *));
			sp->type = type;
			printf("SP NAME2: %s, %d\n", sp->name, type);
			return sp;
		}

		if(++sp >= symtab+NHASH) sp = symtab;
	}

	fputs("symbol table overflow\n", stderr);
	abort(); // Table full
}

void addlevel(int lineno, int placement, char * word) {
	printf("add level\n");
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

void addlabel(int lineno, char * word) {
	printf("add label\n");
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

char * leveltojson(symbol * sym) {
	if(sym->reflist[0].level == NULL) {
		return "";
	}
	char * jsondata = malloc(sizeof(100));
	sprintf(jsondata, "{\"%s\":\"%s\",\"%s\":%d}", "name", strdup(sym->name), "placement", sym->reflist[0].level->placement);
	printf("Level JSON Data: %s\n", jsondata);
	return jsondata;
}

char * labeltojson(symbol * sym) {
	printf("LabelToJSON: %s\n", strdup(sym->name));
	if(sym->reflist[0].label == NULL) {
		return "";
	}
	char * jsondata = malloc(sizeof(100));
	snprintf(jsondata, sizeof(jsondata), "{\"%s\":\"%s\"}", "name", strdup(sym->name));
	return jsondata;
}
