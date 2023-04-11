%{

/*
 * This is the parser for Pamx.
 * Author: Daniel Weninger
 * Last Modified: 11/10/2022
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "semantics.h"

extern int yylex();
extern int yylineno();
extern int yytext();
int printFlag = 0;
int levels = 0;
char * out_path;
char * level_db_path;
%}

%union {
	int intval;
	double floatval;
	char * strval;
	char ** strlist;
	int subtok;
}

%type <strval> id
%type <intval> res
%type <intval> set_res
%type <intval> op_var
%type <intval> op
%type <strval> label_name
%type <strval> level_name
%type <strval> labels
%type <strlist> label_list
%type <strval> file
%type <strval> user
%type <strval> stmt

%token <strval> ID
%token <intval> LEVELLIT
%token <strval> USERASSIGN
%token <strval> LEVEL
%token <strval> LABEL
%token <subtok> CMP


%left '(' ')' '[' ']'
%left ','
%left CMP SET
%left LEVEL LABEL FILEASSIGN USERASSIGN
%left ASSIGN

%start stmt_list

%%

stmt_list	: 	stmt_list stmt
 			|	stmt							{};
stmt		:	USERASSIGN level_name ASSIGN user';'				{ do_user_assign_level(out_path, $2, $4); free($2); free($4);}
			|	USERASSIGN level_name label_list ASSIGN user';'		{ do_user_assign_level(out_path, $2, $5); do_user_assign_labels(out_path, $3, $5); free($2); free($3); free($5); }
			|	USERASSIGN label_list ASSIGN user';'				{ do_user_assign_labels(out_path, $2, $4); free($2); free($4); }
			|	FILEASSIGN level_name ASSIGN file';'				{ do_file_assign_level(out_path, $2, $4); free($2); free($4); }
			|   FILEASSIGN level_name label_list ASSIGN file';'		{ do_file_assign_level(out_path, $2, $5); do_file_assign_labels(out_path, $3, $5); free($2); free($3); free($5); }
			|	FILEASSIGN label_list ASSIGN file';'				{ do_file_assign_labels(out_path, $2, $4); free($2); free($4); }
			|	LABEL label_name';' 								{ do_define_label($2); }
			|	LEVEL level_name op';'								{ do_define_level($2, $3, level_db_path); };
label_list	:	'['labels']'										{ $$ = do_label_list($2); };
labels		:	label_name											{ $$ = do_concat_labels($1); }
			|	label_name',' labels								{ $$ = do_concat_labels($1); };	
user		:	id													{ $$ = $1; };
file		:	id													{ $$ = $1; };
level_name	:	id													{ $$ = $1; };
label_name 	:	id													{ $$ = $1; };	
op	 		:	'('set_res')'										{ $$ = $2; }
   			|	'('op_var')'										{ $$ = $2; };	
op_var		:	CMP id												{ $$ = do_comp($1, $2); };
set_res		:	SET res												{ $$ = do_set($2, level_db_path); };
res			:	LEVELLIT											{ $$ = $1; };
id 			: 	ID													{ $$ = $1; };

%%

int main(int ac, char ** av) {	
	// Section used for batch testing: not working
	// Use batch testing tool instead
	#ifdef TEST
	if(ac != 2 || (strcmp(av[1], "-p") != 0 && strcmp(av[1], "-k") != 0)) {
		fprintf(stderr, "usage: %s (-p | -k)\n", av[0]);
		exit(EXIT_FAILURE);
	}
	printf("Starting Parser Unit Tests...\n\n");
	for(int i = 1; i < 12; i++) {
		printf("Starting test %d:\n", i);
		extern FILE * yyin;
		char current_file_name[64];
		sprintf(current_file_name, "./test_files/test%d", i);
		printf("current_file_name %s\n", current_file_name);
		if((yyin = fopen(current_file_name, "r")) == NULL) {
			perror(av[1]);
			exit(1);
		}
			
		if(!yyparse()) {
			printf("Test %d worked.\n", i);
		} else {
			printf("Test %d failed.\n", i);
		}
		levels = 0;
		fclose(yyin);
	}

	#else
	if(ac != 5 || (strcmp(av[4], "-p") != 0 && strcmp(av[4], "-k") != 0)) {
		fprintf(stderr, "usage: %s <file_in> <file_out> <level_db_out> (-p | -k)\n", av[0]);
		exit(EXIT_FAILURE);
	}

	extern FILE * yyin;
	if((yyin = fopen(av[1], "r")) == NULL) {
		perror(av[1]);
		exit(1);
	}

	if(strcmp(av[4], "-p") == 0) {
		printFlag = 1;
		out_path = av[2];
		FILE * outFile = fopen(out_path, "w+");
		if(!outFile) {
			fprintf(stderr, "Out file path invalid.\n");
			exit(EXIT_FAILURE);
		}
		level_db_path = av[3];
		FILE * levelDBFile = fopen(level_db_path, "w+");
		if(!levelDBFile) {
			fprintf(stderr, "Level database output path is invalid.\n");
			exit(EXIT_FAILURE);
		}
		fprintf(outFile, "");
		fflush(outFile);
		fprintf(levelDBFile, "");
		fflush(levelDBFile);
	} else {
		printf("Kernel reading not yet implemented.\n");
		exit(EXIT_FAILURE);	
	}
	
	if(!yyparse()) {
		printf("Policy parser worked.\n");
	} else {
		printf("Policy parser failed.\n");
	}
	#endif
}

void yyerror(char * msg) {
	printf("%d: %s at '%s'\n", yylineno, msg, yytext);
}
