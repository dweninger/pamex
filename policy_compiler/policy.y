%{
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "semantics.h"

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

UserAssign	:	USERASSIGN LevelName '-''>' User ';'		{ $$ = doUserAssignLevel($2, $5); free($2); free($5);};
UserAssign	:	USERASSIGN LevelName LabelList '-''>' User ';'	{ $$ = doUserAssignLevel($2, $6); doUserAssignLabels($3, $6); free($2); free($3); free($6); };
FileAssign	:	FILEASSIGN LevelName '-''>' File ';'		{ $$ = doFileAssignLevel($2, $5); free($2); free($5); };
FileAssign	:   	FILEASSIGN LevelName LabelList  '-''>' File ';'	{ $$ = doFileAssignLevel($2, $6); doFileAssignLabels($3, $6) free($2); };
User		:	Id						{ $$ = $1; };
File		:	Id						{ $$ = $1; };
LabelList	:	'[' Labels ']'					{ $$ = $2; };
Labels		:	LabelName					{ $$ = doLabelList($1, ""); };
Labels		:	LabelName ',' Labels				{ $$ = doLabelList($1, $3); };	
DefLabel	:	LABEL LabelName ';'				{ $$ = doDefineLabel($2); };
DefLevel	:	LEVEL LevelName Op ';'				{ $$ = doDefineLevel($2, $3); };
LevelName	:	Id						{ $$ = $1; };
LabelName 	:	Id						{ $$ = $1; };	
Op	 	:	'(' SetRes ')'					{ $$ = $2; };
Op   		:	'(' OpVar ')'					{ $$ = $2; };	
OpVar		:	CMP Id						{ $$ = doComp('%d', $2); };
SetRes		:	SET Res						{ $$ = doSet($2); };
Res		:	LEVELLIT					{ $$ = $1; };
Id 		: 	ID						{ $$ = $1; };

%%
