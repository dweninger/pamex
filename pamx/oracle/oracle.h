#include <stdlib.h>
#include <stdio.h>

int getUserLevel(FILE * procFile, FILE * levelDB);
char ** getUserLabels(FILE * procFile);
int getFileLevel(char * targetedFilePath, FILE * levelDBFile);
char ** getFileLabels(char * targetedFilePath);
int containsLabels(char ** refLabels, char ** userLabels);
void getxattrErrorPrints();
