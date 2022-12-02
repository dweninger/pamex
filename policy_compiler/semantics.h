void yyerror(char * s);

void doUserAssignLevel(char * levelName, char * user);
void doUserAssignLabels(char ** labelList, char * user);
void doFileAssignLevel(char * levelName, char * file);
void doFileAssignLabels(char ** labelList, char * file);
char ** doLabelList(char * labels);
char * doConcatLabels(char * label, char * labelList);
void doDefineLevel(char * levelName, int placement);
void doDefineLabel(char * labelName);
int doComp(double op, char * id);
int doSet(char * res);


