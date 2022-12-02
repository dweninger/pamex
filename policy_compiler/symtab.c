#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static unsigned symhash(char * sym) {
	unsigned int hash = 0;
	unsigned c;

	while(c = *sym++) hash = hash*9 ^ c;
	return hash;
}

struct Symbol * lookup(char * sym, int type) {
	struct Symbol * sp = &symtab[symhash(sym)%NHASH];
	int scount = NHASH;

	while(--scount >= 0) {
		if(sp->name && !strcasecmp(sp->name, sym)) return sp;

		if(!sp->name) {
			sp->name = strdup(sym);
			sp->reflist = 0;
			sp->type = type;
			return sp;
		}

		if(++sp >= symtab+NHASH) sp = symtab;
	}

	fputs("symbol table overflow\n", stderr);
	abort(); // Table full
}

void addlevel(int lineno, int placement, char * word) {
	struct LevelRef * lr;
	struct Symbol * sp = lookup(word, LEVEL);
	
	// reference found on same line
	if(sp->reflist &&
	   sp->reflist->lineno == lineno) return; 
	lr = malloc(sizeof(struct LevelRef *));
	if(!lr) { fputs("out of space\n", stderr); abort(); }
	lr->next = sp->reflist;
	lr->lineno = lineno;
	lr->placement = placement;
	sp->reflist = lr;
}

void addlabel(int lineno, char * word) {
	struct LabelRef * lr;
	struct Symbol * sp = lookup(word, LABEL);
	
	// reference found on same line
	if(sp->reflist &&
	   sp->reflist->lineno == lineno) return; 
	lr = malloc(sizeof(struct LabelRef *));
	if(!lr) { fputs("out of space\n", stderr); abort(); }
	lr->next = sp->reflist;
	lr->lineno = lineno;
	sp->reflist = lr;
}

void adduser(int lineno, char * word) {
	struct UserRef * ur;
	struct Symbol * sp = lookup(word, USER_NAME);
	
	// reference found on same line
	if(sp->reflist &&
	   sp->reflist->lineno == lineno) return; 
	ur = malloc(sizeof(struct UserRef *));
	if(!ur) { fputs("out of space\n", stderr); abort(); }
	ur->next = sp->reflist;
	ur->lineno = lineno;
	sp->reflist = ur;
}

void addfile(int lineno, char * word) {
	struct FileRef * fr;
	struct Symbol * sp = lookup(word, FILE_NAME);
	
	// reference found on same line
	if(sp->reflist &&
	   sp->reflist->lineno == lineno) return; 
	fr = malloc(sizeof(struct FileRef *));
	if(!fr) { fputs("out of space\n", stderr); abort(); }
	fr->next = sp->reflist;
	fr->lineno = lineno;
	sp->reflist = fr;
}

char * leveltojson(struct Symbol * sym) {
	if(sym->reflist[0]->type != LEVEL) {
		return "";
	}
	char * jsondata = malloc(sizeof(100));
	snprintf(jsondata, sizeof(jsondata), "{\"%s\" : \"%s\" \"%s\" : %d}", "name", sym->name, "placement", sym->reflist[0]->placement);
	return jsondata;
}

char * labeltojson(struct Symbol * sym) {
	if(sym->reflist[0]->type != LABEL) {
		return "";
	}
	char * jsondata = malloc(sizeof(100));
	snprintf(jsondata, sizeof(jsondata), "{\"%s\" : \"%s\"}", "name", sym->name);
	return jsondata;
}
