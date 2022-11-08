%{
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "policy.h"

extern int yylex();
%}

%union {
	int intval;
	double floatval;
	char * strval;
	int subtok;
}

%type <strval> Id
%type <strval> Res
%type <strval> SetRes
%type <strval> OpVar
%type <strval> Op
%type <strval> LabelName
%type <strval> LevelName
%type <strval> DefLevel
%type <strval> DefLabel
%type <strval> Labels
%type <strval> LabelList
%type <strval> File
%type <strval> User
%type <strval> FileAssign
%type <strval> UserAssign

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

%%

UserAssign	:	USERASSIGN LevelName '-''>' User ';'	{ $$ = doUserAssign($2, $5); }
	   	|	USERASSIGN LevelName LabelList '-''>' User ';'	{ $$ = doUserAssignWithList($2, $6, $3); }
FileAssign	:	FILEASSIGN LevelName '-''>' File ';'	{ $$ = doFileAssign($2, $5); }
	   	|	FILEASSIGN LevelName LabelList  '-''>' File ';'	{ $$ = doFileAssignWithList($2, $6, $3); }
User		:	Id			{ $$ = $1; }
File		:	Id			{ $$ = $1; }
LabelList	:	'[' Labels ']'		{ $$ = $2; }
Labels		:	LabelName		{ $$ = doLabelList($1); }
       		|	LabelName ',' Labels	{ $$ = doLabelListMultiple($1, $3); }	
DefLabel	:	LABEL LabelName Op ';'	{ $$ = doDefineLabel($2, $3); }
DefLevel	:	LEVEL LevelName Op ';'	{ $$ = doDefineLevel($2, $3); }
LevelName	:	Id		{ $$ = $1; }
LabelName 	:	Id		{ $$ = $1; }
Op	 	:	'(' SetRes ')'	{ $$ = $2; }
   		|	'(' OpVar ')'	{ $$ = $2; }	
OpVar		:	CMP Id		{ $$ = doComp('%d', $2); }
SetRes		:	SET Res		{ $$ = doSet($2); }
Res		:	LEVELLIT	{ $$ = $1; }
Id 		: 	ID		{ $$ = $1; } 

%%
