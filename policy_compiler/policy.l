%option noyywrap nodefault yylineno

%{

/*
 * This is the lexer for the SimpleFlow policy compiler.
 * Author: Daniel Weninger
 * Last Modified: 10/25/2022
 */

#include <stdarg.h>
#include <string.h>
#include "policy.tab.h"

void yyerror(const char * s);
%}

ws		[ \t]+
comment 	#.*
nl		\n
id		[A-Za-z][-_A-Za-z0-9]+
letter	 	[A-Za-z]
digit 		[0-9]

%%

"level"		{ return LEVEL; }
"label"		{ return LABEL; }
"set"		{ return SET; }
"file-assign"	{ return FILEASSIGN; }
"user-assign"	{ return USERASSIGN; }
"restricted"	{ yylval.intval = 1; return LEVELLIT; }
"unrestricted"	{ yylval.intval = 0; return LEVELLIT; }
"<"		{ yylval.subtok = 1; return CMP; }
">"		{ yylval.subtok = 2; return CMP; }
";" |		
"," |		
"(" |		
")" |		
"[" |		
"]"		{ return yytext[0]; }		
"->"		{ return ASSIGN; }
{id}		{ yylval.strval = strdup(yytext); return ID; }
{comment}	{ yylval.strval = strdup(yytext); };
{ws}		{ };
{nl}		{ }; 
.		{ perror("Error in Lex"); }

%%

int main(int argc, char **argv) {
	if(argc > 1) {
		if(!(yyin = fopen(argv[1], "r"))){
			perror(argv[1]);
			return 1;
		}	
	}

	yylex();
}

void yyerror(const char *s)
{
             fprintf(stderr, "%d: %s\n", yylineno, s);
}
