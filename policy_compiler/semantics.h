void yyerror(char * s);

void doUserAssignLevel(char * levelName, char * user);
void doUserAssignLabels(char ** labelList, char * user);
void doFileAssignLevel(char * levelName, char * file);
void doFileAssignLabels(char ** labelList, char * file);
char ** doLabelList(char * labels);
char * doConcatLabels(char * label);
void doDefineLevel(char * levelName, int placement);
void doDefineLabel(char * labelName);
int doComp(int op, char * id);
int doSet(int res);


