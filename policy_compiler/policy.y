%{

/*
 * This is the parser for the SimpleFlow policy compiler.
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
%}

%union {
	int intval;
	double floatval;
	char * strval;
	int subtok;
}

%type <strval> id
%type <strval> res
%type <strval> set_res
%type <strval> op_var
%type <strval> op
%type <strval> label_name
%type <strval> level_name
%type <strval> labels
%type <strval> label_list
%type <strval> file
%type <strval> user
%type <strval> stmt

%token <strval> ID
%token <strval> LEVELLIT
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
 		|	stmt							;
stmt		:	USERASSIGN level_name '-''>' user ';'			{ doUserAssignLevel($2, $5); free($2); free($5);}
		|	USERASSIGN level_name label_list '-''>' user ';'	{ doUserAssignLevel($2, $6); doUserAssignLabels($3, $6); free($2); free($3); free($6); }
		|	FILEASSIGN level_name '-''>' file ';'			{ doFileAssignLevel($2, $5); free($2); free($5); }
		|   	FILEASSIGN level_name label_list  '-''>' file ';'	{ doFileAssignLevel($2, $6); doFileAssignLabels($3, $6); free($2); }
		|	LABEL label_name ';' 					{ doDefineLabel($2); }
		|	LEVEL level_name op ';'					{ doDefineLevel($2, $3); };

label_list	:	'[' labels ']'						{ $$ = $2; };
labels		:	label_name						{ doLabelList($1, ""); }
		|	label_name ',' labels					{ doLabelList($1, $3); };	
user		:	id							{ $$ = $1; };
file		:	id							{ $$ = $1; };
level_name	:	id							{ $$ = $1; };
label_name 	:	id							{ $$ = $1; };	
op	 	:	'(' set_res ')'						{ $$ = $2; };
op   		:	'(' op_var ')'						{ $$ = $2; };	
op_var		:	CMP id							{ doComp($1, $2); };
set_res		:	SET res							{ doSet($2); };
res		:	LEVELLIT						{ $$ = $1; };
id 		: 	ID							{ $$ = $1; };

%%

int main(int ac, char ** av) {	

	#ifdef TEST
	printf("Starting Parser Unit Tests...\n\n");
	for(int i = 1; i < 12; i++) {
		extern FILE * yyin;
		char current_file_name[64];
		sprintf(current_file_name, "./test_files/test%d", i);
		printf("Current file name: %s\n", current_file_name);
		if((yyin = fopen(current_file_name, "r")) == NULL) {
			perror(av[1]);
			exit(1);
		}
	
		if(!yyparse()) {
			printf("Test %d worked.\n", i);
		} else {
			printf("Test %d failed.\n", i);
		}

	}

	#else
	extern FILE * yyin;
	if(ac > 1 && (yyin = fopen(av[1], "r")) == NULL) {
		perror(av[1]);
		exit(1);
	}

	if(!yyparse()) {
		printf("Policy parse worked.\n");
	} else {
		printf("Policy parse failed.\n");
	}
	#endif

}

void yyerror(char * msg) {
	printf("%d: %s at '%s'\n", yylineno, msg, yytext);
}
