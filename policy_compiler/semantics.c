#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "semantics.h"

void doUserAssignLevel(char * levelName, char * user) {
	printf("doUserAssignLevel\n");
}

void doUserAssignLabels(char * labelList, char * user) {	
	printf("doUserAssignLabels\n");
}

void doFileAssignLevel(char * levelName, char * file) {
	printf("doFileAssignLevel\n");
}

void doFileAssignLabels(char * labelList, char * file) {
	printf("doFileAssignLabels\n");
}

void doLabelList(char * label, char * labelList) {
	printf("doLabelList\n");
}

void doDefineLevel(char * levelName, char * op) {
	printf("doDefineLevel\n");
}

void doDefineLabel(char * labelName) {
	printf("doDefineLabel\n");
}

void doComp(double op, char * id) {
	printf("doComp\n");
}

void doSet(char * res) {
	printf("doSet\n");
}

