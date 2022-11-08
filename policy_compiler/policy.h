void yyerror(const char * s);

char * doUserAssign(char * levelName, char * user, ...);
char * doFileAssign(char * levelName, char * file, ...);
char * doLabelList(char * label, ...);
char * doDefineLevel(char * levelName, char * op);
char * doDefineLabel(char * labelName);
char * doComp(double op, char * id);
char * doSet(char * res);
