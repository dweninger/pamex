%{

/*
 * This is the lexer for the SimpleFlow policy compiler.
 * Author: Daniel Weninger
 * Last Modified: 10/25/2022
 */

enum yytokentype {
	LEVEL = 258,
	LABEL = 259,
	SET = 260,
	FILEASSIGN = 261,
	USERASSIGN = 262,
	LevelLit = 263,
	LT = 264,
	GT = 265,
	ASSIGN = 266,
	ID = 267,
};

int yylval;
#include <string.h>
int lineno = 1;
void yyerror(char *s);

%}

ws		[ \t]+
comment 	#.*
nl		\n
id		[A-Za-z][-_A-Za-z0-9]+
letter	 	[A-Za-z]
digit 		[0-9]

%%

level		{ return LEVEL; }
label		{ return LABEL; }
set		{ return SET; }
file-assign	{ return FILEASSIGN; }
user-assign	{ return USERASSIGN; }
restricted	{ return LevelLit; }
unrestricted	{ return LevelLit; }
"<"		{ return LT; }
">"		{ return GT; }
";"		{ return ';'; }
","		{ return ','; }
"("		{ return '('; }
")"		{ return ')'; }
"["		{ return '['; }
"]"		{ return ']'; }
"->"		{ return ASSIGN; }
{id}		{ return ID; }
{comment}	{ };
{ws}		{ };
{nl}		{ lineno = lineno + 1; } 
.		{ return yytext[0]; }

%%

void yyerror(char *s) {
	printf("%d: %s at %s\n", lineno, s, yytext);
}

int main(int argc, char **argv) {
	int tok;

	while(tok = yylex()) {
		printf("%d", tok);
		if(tok == ID) {
			printf(" = %d\n", yylval);
		} else {
			printf("/n");
		}
	}
}

int yywrap() {
	return 1;
}
