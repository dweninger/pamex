#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/xattr.h>

#include "semantics.h"
#include "symtab.h"

void shiftLevelPlacements(int pos);


/**
 * TODO
 */
void doUserAssignLevel(char * levelName, char * user) {
	printf("doUserAssignLevel\n");
}

/**
 * TODO
 */
void doUserAssignLabels(char ** labelList, char * user) {	
	printf("doUserAssignLabels\n");
}

/**
 * doFileAssignLevel - Add the level JSON to the extended attributes of the specified
 * 	file
 * levelName  the name of the level that will be added to the extended attributes
 * file  the path of the file that the level will be assigned to
 */
void doFileAssignLevel(char * levelName, char * file) {
	struct Symbol * sym = lookup(levelName, LEVEL);
	char * leveljson = leveltojson(sym);
	// set file extended attribute to have level JSON value
	// fsc = file security classification
	if(setxattr(file, "security.fsc.level", leveljson, strlen(leveljson), 0) == -1) {
		exit(EXIT_FAILURE);
	}
}

/**
 * doFileAssignLabels - Add the label JSON to the extended attributes of the specified
 * 	file
 * labelList  a list of strings that contain the names of labels
 * file  the path of the file that the labels will be assigned to
 */
void doFileAssignLabels(char ** labelList, char * file) {
	char * labeljson = malloc(sizeof(1024));
	// build label JSON
	strcat(labeljson, "[");
	int labelListSize = sizeof(labelList - sizeof(char *));
	for(int i = 0; i < labelListSize; i++) {
		strcat(labeljson, labeltojson(lookup(labelList[i], LABEL)));
		if((i + 1) < labelListSize) {
			strcat(labeljson, ", ");
		}
	}
	strcat(labeljson, "]");
	// set file extended attribute to have label JSON value
	// fsc = file security classification
	if(setxattr(file, "security.fsc.labels", labeljson, strlen(labeljson), 0) == -1) {
		exit(EXIT_FAILURE);
	}
}

/**
 * doLabelList - Create a char** struct that contains a list of the names of labels
 * label 
 * labelList
 * returns labelArr
 */
char ** doLabelList(char * labelList) {
	char ** labelArr;
	labelArr = (char**)malloc(sizeof(char *));
	int index = 1;
	char * token = strtok(labelList, ", ");
	labelArr[0] = strdup(token);
	while(token != NULL) {
		labelArr = (char **)realloc(labelArr, (index + 1) * sizeof(char *));
		labelArr[index] = strdup(token);
		token = strtok(NULL, ", ");
		index ++;
	}
	return labelArr;
}

/**
 * doConcatLabels - concatenate a label to the beginning of a string of labels that
 * 	represent a label list
 * label  the new label to be added
 * labelList  the existing string representing a list of labels
 */
char * doConcatLabels(char * label, char * labelList) {
	char * labels = malloc( sizeof(char *));
	strcat(labels, strdup(label));
	strcat(labels, strdup(labelList));
	return labels;
}

/**
 * doDefineLevel - Define either a pre-existing or a new level and add it to the
 * 	symbol table and the levelplacements array
 * levelName  the name of the level to be added
 * placement  the hierarchy ranking of the level to be added
 */
void doDefineLevel(char * levelName, int placement) {
	// Find if level var already exists
	struct Symbol * sym = lookup(levelName, LEVEL);
	// check if levels need to shift placements
	if(levelplacements[placement]) {
		shiftLevelPlacements(placement);
	}
	if(sym) {
		// level is being re-defined. allow?
	} else {
		// new level being added to symtab
		addlevel(0, placement, levelName);
		sym = lookup(levelName, LEVEL);
	}	
	// add level to placements array
	levelplacements[placement] = sym;
}

/**
 * shiftLevelPlacements - In the list of level placements, shift all of the levels
 * 	at the param's level and above up 1 to make room for new level placement
 * pos  the position that the new level should be at
 */
void shiftLevelPlacements(int pos) {
	int i = 0;
	for(i = sizeof(levelplacements - sizeof(int)); i > pos; i--) {
		levelplacements[i] = levelplacements[i - 1];
	}
	levelplacements[i] = NULL;
}

/**
 * doDefineLabel - add a label name to the symbol table
 * labelName  name of a new label 
 */
void doDefineLabel(char * labelName) {
	struct Symbol * sym = lookup(labelName, LABEL);
	if(sym){
		// label is being re-defined. Don't allow
		exit(EXIT_FAILURE);
	} else {
		// add new label into symbol table
		addlabel(0, labelName);
	}
}

/**
 * doComp - return the placement number of either 1 less or 1 more than
 * 	the level param
 * op  the operation, either > or <, 
 *     indicating whether the level should be 1 placement higher or lower
 * id  the id of the level that the operator is refering to
 * returns  int containing the level placement
 */
int doComp(double op, char * id) {
	int curPlacement = -1;
	struct Symbol * sym = lookup(id, LEVEL);
	if(op == 1) {	
		// <
		int tempPlacement = sym->reflist[0]->placement - 1;
		// TODO: check tempPlacement is not below 1
		curPlacement = tempPlacement;
	} else if(op == 2) {
		// >
		curPlacement = sym->reflist[0]->placement + 1;
	} else {
		printf("comparison operator error\n");
	}

	return curPlacement;
}

/**
 * doSet - return the placement of either 0 or 1 depending on if the
 * 	param is unrestricted or restricted
 * res  checks if the level is restricted or unrestricted
 * returns  int containing the level placement
 */
int doSet(char * res) {
	int curPlacement = -1;
	// check if level is unrestricted or restricted
	if(strcmp(res, "unrestricted") == 0) {
		// unrestricted placement = 0
		curPlacement = 0;
	} else if (strcmp(res, "restricted") == 0) {
		// restricted placement = 1
		curPlacement = 1;
	} else {
		printf("error in doSet\n");
	}
	return curPlacement;
}


