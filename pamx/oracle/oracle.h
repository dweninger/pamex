#include <stdlib.h>
#include <stdio.h>

int getUserLevel(FILE * procFile);
char ** getUserLabels(FILE * procFile);
int getFileLevel(char * targetedFilePath);
char ** getFileLabels(char * targetedFilePath);
int containsLabels(char ** refLabels, char ** userLabels);
void getxattrErrorPrints();
