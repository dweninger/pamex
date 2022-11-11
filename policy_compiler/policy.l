%option noyywrap nodefault yylineno

%{

/*
 * This is the lexer for the SimpleFlow policy compiler.
 * Author: Daniel Weninger
 * Last Modified: 11/10/2022
 */

#include <stdarg.h>
#include <string.h>
#include "policy.tab.h"

void yyerror(char * msg);
%}

ws		[ \t]+
comment 	#.*
nl		\n
id		[\.\/A-Za-z][-_\/\.A-Za-z0-9]*
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
.		{ yyerror("Error in Lex\n"); }

%%

