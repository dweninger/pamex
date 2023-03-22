#include <stdlib.h>
#include <stdio.h>

int getUserLevel(char * procFilePath, char * levelDBPath);
char ** getUserLabels(char * procFilePath);
int getFileLevel(char * targetedFilePath, char * levelDBPath);
char ** getFileLabels(char * targetedFilePath);
int containsLabels(char ** refLabels, char ** userLabels);
void getxattrErrorPrints();
