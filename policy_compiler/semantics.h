void yyerror(char * s);

void doUserAssignLevel(char * levelName, char * user);
void doUserAssignLabels(char * labelList, char * user);
void doFileAssignLevel(char * levelName, char * file);
void doFileAssignLabels(char * labelList, char * file);
void doLabelList(char * label, char * labelList);
void doDefineLevel(char * levelName, char * op);
void doDefineLabel(char * labelName);
void doComp(double op, char * id);
void doSet(char * res);
