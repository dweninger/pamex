#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "semantics.h"

char * doUserAssignLevel(char * levelName, char * user) {
	return levelName;
}

char * doUserAssignLabels(char * labelList, char * user) {
	return labelList;
}

char * doFileAssignLevel(char * levelName, char * file) {
	return levelName;
}

char * doFileAssignLabels(char * labelList, char * file) {
	return labelList;
}

char * doLabelList(char * label, char * labelList) {
	return label;
}

char * doDefineLevel(char * levelName, char * op) {
	return levelName;
}

char * doDefineLabel(char * labelName) {
	return labelName;
}

char * doComp(double op, char * id) {
	return id;
}

char * doSet(char * res) {
	return res;
}

