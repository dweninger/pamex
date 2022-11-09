void yyerror(char * s);

char * doUserAssignLevel(char * levelName, char * user);
char * doUserAssignLabels(char * labelList, char * user);
char * doFileAssignLevel(char * levelName, char * file);
char * doFileAssignLabels(char * labelList, char * file);
char * doLabelList(char * label, char * labelList);
char * doDefineLevel(char * levelName, char * op);
char * doDefineLabel(char * labelName);
char * doComp(double op, char * id);
char * doSet(char * res);
