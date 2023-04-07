/**
 * This is the sematics file for Pamx.
 * These functions are called by the yacc file and are used to interpret the syntax
 * 	and output the file and user definiton file 
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/xattr.h>

#include "semantics.h"
#include "symtab.h"

extern int levels; // Amount of levels that have been defined
extern symbol * level_placements[1024]; // An ordered list of level symbols by placement
extern symbol symtab[NHASH]; // The symbol table holding all of the variable info
extern int print_flag; // Whether or not the program should print
char * g_label_list; // 

void Finish() {
	//levels = 0;	
	//print_flag = -1;
	//free(g_label_list);
}

/**
 * do_user_assign_level - write the level information for a particular user to the out file
 * level_name  the name of the level that is being written
 * user  the name of the user that is being written
 */
void do_user_assign_level(char * file_path, char * level_name, char * user) {
	symbol * sym = lookup(level_name, LEVEL);
	char * level_data = format_level_data(sym);
	
	FILE * out_file = fopen(file_path, "a");
	fprintf(out_file, "USER_LEVEL %s %s\n", user, level_data);
	fclose(out_file);
}

/**
 * do_user_assign_labels - write the label information for a particular user to the out file 
 * label_list  a string list of labels that are being written
 * user  the name of the user that is being written
 */
void do_user_assign_labels(char * file_path, char ** label_list, char * user) {	
	int label_list_size = sizeof(label_list) / sizeof(label_list[0]);
	int index = 0;
	char * cur = label_list[index];
	while(cur != NULL) {
		FILE * out_file = fopen(file_path, "a");
		fprintf(out_file, "USER_LABELS %s %s\n", user, label_list[index]);
		fclose(out_file);

		index++;
		cur = label_list[index];
	}
}

/**
 * do_file_assign_level - Add the level JSON to the extended attributes of the specified
 * 	file
 * level_name  the name of the level that will be added to the extended attributes
 * file  the path of the file that the level will be assigned to
 */

void do_file_assign_level(char * file_path, char * level_name, char * file) {
	symbol * sym = lookup(level_name, LEVEL);
	char * level_data = format_level_data(sym);
	
	FILE * out_file = fopen(file_path, "a");
	fprintf(out_file, "FILE_LEVEL %s %s\n", file, level_data);
	fclose(out_file);
}

/**
 * do_file_assign_labels - Add the label JSON to the extended attributes of the specified
 * 	file
 * label_list  a list of strings that contain the names of labels
 * file  the path of the file that the labels will be assigned to
 */
void do_file_assign_labels(char * file_path, char ** label_list, char * file) {
	int label_list_size = sizeof(label_list) / sizeof(label_list[0]);
	int index = 0;
	char * cur = label_list[index];
	while(cur != NULL) {
		FILE * out_file = fopen(file_path, "a");
		fprintf(out_file, "FILE_LABELS %s %s\n", file, label_list[index]);
		fclose(out_file);

		index++;
		cur = label_list[index];
	}
}

/**
 * do_label_list - Create a char** struct that contains a list of the names of labels
 * label 
 * label_list
 * returns label_arr
 */
char ** do_label_list(char * label_list) {
	char ** label_arr;
	label_arr = (char**)malloc(sizeof(char *));
	int index = 0;
	char * token = strtok(label_list, ", ");
	label_arr[0] = strdup(token);
	while(token != NULL) {
		label_arr = (char **)realloc(label_arr, (index + 1) * sizeof(char *));
		label_arr[index] = strdup(token);
		token = strtok(NULL, ", ");
		index ++;
	}
	g_label_list = "";
	g_label_list = malloc(sizeof(char *));
	return label_arr;
}

/**
 * do_concat_labels - concatenate a label to the beginning of a string of labels that
 * 	represent a label list
 * label  the new label to be added
 * label_list  the existing string representing a list of labels
 */
char * do_concat_labels(char * label) {
	if(strlen(g_label_list) > 0) {
		strcat(g_label_list, ", ");
	}
	strcat(g_label_list, label);

	char * labels = malloc(sizeof(char *));
	labels = strdup(g_label_list);
	return labels;
}

/**
 * do_define_level - Define either a pre-existing or a new level and add it to the
 * 	symbol table and the level_placements array
 * level_name  the name of the level to be added
 * placement  the hierarchy ranking of the level to be added
 */
void do_define_level(char * level_name, int placement) {	
	// Find if level in symtab already exists or create new symbol for level
	symbol * sym = lookup(level_name, LEVEL);
	
	// check if levels need to shift placements
	if(level_placements[placement] && placement != 0) {
		shift_level_placements(placement);
	} else if (level_placements[placement] && placement == 0) {
		sym->name = strdup(level_name);
		levels++;
		return;
	} 
	
	if(sym->new_sym) {
		// new level being added to symtab
		add_level(0, placement, level_name);
		sym = lookup(level_name, LEVEL);
	} else {
		return;
		// level is being re-defined. allow?
	}
	// add level to placements array
	level_placements[placement] = sym;
	levels++;
}

/**
 * shift_level_placements - In the list of level placements, shift all of the levels
 * 	at the param's level and above up 1 to make room for new level placement
 * pos  the position that the new level should be at
 */
void shift_level_placements(int pos) {
	int i;
	for(i = levels - 1; i > pos - 1; i--) {
		level_placements[i + 1] = level_placements[i];
		level_placements[i + 1]->ref_list[0].level->placement++;
	}
}

/**
 * print_level_placements - prints all of the level placements in the level_placements array.
 * 	(used for debugging)
 */
void print_level_placements() {
	FILE * levels_fp = fopen("../data/level_order.txt", "w");
	int i = 0;
	while(level_placements[i] && strcmp(level_placements[i]->name, "") != 0){
		fprintf(levels_fp, "%s:%d\n", level_placements[i]->name, i);
		i++;
	}
	fclose(levels_fp);
}

/**
 * do_define_label - add a label name to the symbol table
 * label_name  name of a new label 
 */
void do_define_label(char * label_name) {
	symbol * sym = lookup(label_name, LABEL);
	if(sym->new_sym){
		// add new label into symbol table
		add_label(0, label_name);
	} else {
		// label is being re-defined. Don't allow
		perror("Error: cannot redefine label");
		exit(EXIT_FAILURE);
	}
	g_label_list = malloc(sizeof(char *));
}

/**
 * do_comp - return the placement number of either 1 less or 1 more than
 * 	the level param
 * op  the operation, either > or <, 
 *     indicating whether the level should be 1 placement higher or lower
 * id  the id of the level that the operator is refering to
 * returns  int containing the level placement
 */
int do_comp(int op, char * id) {
	int cur_placement = -1;
	symbol * sym = lookup(id, LEVEL);
	if(op == 1) {	
		// <
		cur_placement = sym->ref_list[0].level->placement;
	} else if(op == 2) {
		// >
		cur_placement = sym->ref_list[0].level->placement + 1;
	} else {
		perror("Comparison operator error\n");
		exit(EXIT_FAILURE);
	}
	return cur_placement;
}

/**
 * do_set - return the placement of either 0 or 1 depending on if the
 * 	param is unrestricted or restricted
 * res  checks if the level is restricted or unrestricted
 * returns  int containing the level placement
 */
int do_set(int res) {
	if(levels == 0) {
		if(res != 0) {
			do_define_level("unrestricted", 0);
		}
	}	
	// check if level is unrestricted or restricted
	if(res != 0 && res != 1) {
		perror("Error setting level\n");
		exit(EXIT_FAILURE);
	}
	// return restricted or unrestricted to be used later for placement
	return res;
}


