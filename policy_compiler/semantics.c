#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/xattr.h>

#include "semantics.h"
#include "symtab.h"

extern int levels;
extern symbol * levelplacements[1024];
extern symbol symtab[NHASH];
extern int printFlag;
char * gLabelList;

void shiftLevelPlacements(int pos);


/**
 * TODO
 */
void doUserAssignLevel(char * levelName, char * user) {
	printf("\ndoUserAssignLevel\n");
	symbol * sym = lookup(levelName, LEVEL);
	char * leveljson = leveltojson(sym);
	
	FILE * outFile = fopen("policy-out.txt", "a");
	fprintf(outFile, "USER_LEVEL %s %s\n", user, leveljson);
	fclose(outFile);
}

/**
 * TODO
 */
void doUserAssignLabels(char ** labelList, char * user) {	
	printf("\ndoUserAssignLabels\n");
	int labelListSize = sizeof(labelList) / sizeof(labelList[0]);
	for(int i = 0; i < labelListSize; i++) {
		printf("labelList[i]: %s\n", labelList[i]);
		FILE * outFile = fopen("policy-out.txt", "a");
		fprintf(outFile, "USER_LABELS %s %s\n", user, labelList[i]);
		fclose(outFile);
	}
}

/**
 * doFileAssignLevel - Add the level JSON to the extended attributes of the specified
 * 	file
 * levelName  the name of the level that will be added to the extended attributes
 * file  the path of the file that the level will be assigned to
 */

void doFileAssignLevel(char * levelName, char * file) {
	printf("\ndoFileAssignLevel\n");
	symbol * sym = lookup(levelName, LEVEL);
	char * leveljson = leveltojson(sym);
	
	FILE * outFile = fopen("policy-out.txt", "a");
	fprintf(outFile, "FILE_LEVEL %s %s\n", file, leveljson);
	fclose(outFile);
	// set file extended attribute to have level JSON value
	// fsc = file security classification
	//if(setxattr(file, "security.fsc.level", leveljson, strlen(leveljson), 0) == -1) {
	//	exit(EXIT_FAILURE);
	//}
}

/**
 * doFileAssignLabels - Add the label JSON to the extended attributes of the specified
 * 	file
 * labelList  a list of strings that contain the names of labels
 * file  the path of the file that the labels will be assigned to
 */
void doFileAssignLabels(char ** labelList, char * file) {
	printf("\ndoFileAssignLabels\n");
	//char * labeljson = malloc(sizeof(1024));
	// build label JSON
	//strcat(labeljson, "[");
	int labelListSize = sizeof(labelList) / sizeof(labelList[0]);
	printf("LABEL LIST SIZE: %d %d\n", sizeof(labelList), sizeof(labelList[0]));
	int index = 0;
	char * cur = labelList[index];
	while(cur != NULL) {
		FILE * outFile = fopen("policy-out.txt", "a");
		fprintf(outFile, "FILE_LABELS %s %s\n", file, labelList[index]);
		fclose(outFile);

		index++;
		cur = labelList[index];
	}
	//	strcat(labeljson, labeltojson(lookup(labelList[i], LABEL)));
	//	if((i + 1) < labelListSize) {
	//		strcat(labeljson, ", ");
	//	}
	//strcat(labeljson, "]");
	
		
	// set file extended attribute to have label JSON value
	// fsc = file security classification
	//if(setxattr(file, "security.fsc.labels", labeljson, strlen(labeljson), 0) == -1) {	
	//	exit(EXIT_FAILURE);
	//}
}

/**
 * doLabelList - Create a char** struct that contains a list of the names of labels
 * label 
 * labelList
 * returns labelArr
 */
char ** doLabelList(char * labelList) {
	printf("\ndoLabelList: %s\n", labelList);
	char ** labelArr;
	labelArr = (char**)malloc(sizeof(char *));
	int index = 0;
	char * token = strtok(labelList, ", ");
	labelArr[0] = strdup(token);
	while(token != NULL) {
		labelArr = (char **)realloc(labelArr, (index + 1) * sizeof(char *));
		labelArr[index] = strdup(token);
		token = strtok(NULL, ", ");
		index ++;
	}
	printf("checklabelArray\n");
	printf("labelArr[1]: %s\n", labelArr[1]);
	gLabelList = "";
	//free(labelList);
	gLabelList = malloc(sizeof(char *));
	return labelArr;
}

/**
 * doConcatLabels - concatenate a label to the beginning of a string of labels that
 * 	represent a label list
 * label  the new label to be added
 * labelList  the existing string representing a list of labels
 */
char * doConcatLabels(char * label) {
	printf("\ndoConcatLabels\n");
	printf("label: %s\n", label);
	//printf("labels: %s\n", labelList);
	
	printf("Label list 1: %s\n", gLabelList);
	if(strlen(gLabelList) > 0) {
		strcat(gLabelList, ", ");
	}
	strcat(gLabelList, label);

	printf("Label list 2: %s\n", gLabelList);
	char * labels = malloc(sizeof(char *));
	labels = strdup(gLabelList);
	//strcat(labels, strdup(label));
	//if(strlen(labelList) > 0) {
	//	strcat(labels, strdup(labelList));
	//}
	
	printf("concated labels: %s\n", labels);
	return labels;
}

/**
 * doDefineLevel - Define either a pre-existing or a new level and add it to the
 * 	symbol table and the levelplacements array
 * levelName  the name of the level to be added
 * placement  the hierarchy ranking of the level to be added
 */
void doDefineLevel(char * levelName, int placement) {	
	printf("\ndoDefineLevel %s %d %d\n", levelName, placement, levels);
	// Find if level in symtab already exists or create new symbol for level
	symbol * sym = lookup(levelName, LEVEL);
	
	printf("shift level placements\n");
	// check if levels need to shift placements
	if(levelplacements[placement]) {
		shiftLevelPlacements(placement);
	}
	
	printf("check new sym\n");
	if(sym->newSym) {
		// new level being added to symtab
		addlevel(0, placement, levelName);
		sym = lookup(levelName, LEVEL);

	} else {
		// level is being re-defined. allow?
	}
	
	printf("assign level placements at placement\n");
	// add level to placements array
	levelplacements[placement] = sym;
	levels ++;
}

/**
 * shiftLevelPlacements - In the list of level placements, shift all of the levels
 * 	at the param's level and above up 1 to make room for new level placement
 * pos  the position that the new level should be at
 */
void shiftLevelPlacements(int pos) {
	printf("\ndoShiftLevelPlacements %d\n", pos);
	int i;
	for(i = levels - 1; i > pos; i--) {
		printf("LEVEL: %d\n", i);
		levelplacements[i]->reflist[0].level->placement = levelplacements[i]->reflist[0].level->placement - 1;
		levelplacements[i] = levelplacements[i - 1];
	}
	levelplacements[i] = NULL;
}

static void printLevelPlacements() {
	for(int i = 0; i < sizeof(levelplacements) / sizeof(levelplacements[0]); i++) {
		printf("LP: [%d, %s]", i, levelplacements[i]->name);
	}
}

/**
 * doDefineLabel - add a label name to the symbol table
 * labelName  name of a new label 
 */
void doDefineLabel(char * labelName) {
	printf("\ndoDefineLabel\n");
	symbol * sym = lookup(labelName, LABEL);
	if(sym->newSym){
		// add new label into symbol table
		addlabel(0, labelName);
	} else {
		// label is being re-defined. Don't allow
		perror("Error: cannot redefine label");
		exit(EXIT_FAILURE);
	}
	gLabelList = malloc(sizeof(char *));
}

/**
 * doComp - return the placement number of either 1 less or 1 more than
 * 	the level param
 * op  the operation, either > or <, 
 *     indicating whether the level should be 1 placement higher or lower
 * id  the id of the level that the operator is refering to
 * returns  int containing the level placement
 */
int doComp(int op, char * id) {
	printf("\ndoComp: %d, %s\n", op, id);
	int curPlacement = -1;
	symbol * sym = lookup(id, LEVEL);
	if(op == 1) {	
		// <
		int tempPlacement = sym->reflist[0].level->placement;
		// TODO: check tempPlacement is not below 1
		curPlacement = tempPlacement;
	} else if(op == 2) {
		// >
		printf("doComp Sym Found: %s\n", sym->name);
		curPlacement = sym->reflist[0].level->placement + 1;
	} else {
		perror("Comparison operator error\n");
		exit(EXIT_FAILURE);
	}
	printf("doComp Placement: %d\n", curPlacement);

	return curPlacement;
}

/**
 * doSet - return the placement of either 0 or 1 depending on if the
 * 	param is unrestricted or restricted
 * res  checks if the level is restricted or unrestricted
 * returns  int containing the level placement
 */
int doSet(int res) {
	printf("\ndoSet: %d\n", res);
	// check if level is unrestricted or restricted
	if(res != 0 && res != 1) {
		perror("Error setting level\n");
		exit(EXIT_FAILURE);
	}
	// return restricted or unrestricted to be used later for placement
	return res;
}


