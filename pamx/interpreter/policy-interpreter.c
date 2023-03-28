/**
 * This is the policy interpreter for Pamx
 * This tool is used to interpret the file and user definiton file 
 * 	and create the targeted users database and file's extended attributes
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <sys/xattr.h>
#include <errno.h>
#include <regex.h>

#include "policy-interpreter.h"

int file_paths_size;
char ** file_paths;
int db_accessed = 0;

int main(int ac, char ** av) {
	printf("Starting policy interpreter tool...\n");

	// Check if there are correct amount of params
	if(ac != 3) {
		fprintf(stderr, "usage: %s <policy_filepath> <dir_of_files>\n", av[0]);
		exit(EXIT_FAILURE);
	}

	// Initiate global variables
	file_paths_size = 0;
	file_paths = (char **)malloc(sizeof(char*));

	// Initiate local variables
	char * path_to_files = strdup(av[2]);
	FILE * file_in = fopen(av[1], "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	// Check if file_in exists
	if(file_in == NULL) {
		fprintf(stderr, "Input file does not exist\n");
		exit(EXIT_FAILURE);
	}

	// Read file_in line by line	
	while((read = getline(&line, &len, file_in)) != -1) {
		// read line data and separate into vars based on space delimiter
		char * assignment_type;
		char * assignee;
		char * assignment_data;
        char * token = strtok(line, " ");

        assignment_type = strdup(token);
		token = strtok(NULL, " ");
		
		assignee = strdup(token);
		token = strtok(NULL, " ");
		
		assignment_data = strdup(token);
		// Remove trailing newline
		assignment_data[strcspn(assignment_data, "\r\n")] = 0;	
		
		// Check which operation to do based on line prefix	
		if (strcmp(assignment_type, "FILE_LEVEL") == 0) {
			assign_level_to_file(assignee, assignment_data, path_to_files);			

		} else if (strcmp(assignment_type, "FILE_LABELS") == 0) {
			assign_label_to_file(assignee, assignment_data, path_to_files);
		
		} else if (strcmp(assignment_type, "USER_LEVEL") == 0) {
			write_user_level_to_db(assignee, assignment_data);
		} else if (strcmp(assignment_type, "USER_LABELS") == 0) {
			write_user_label_to_db(assignee, assignment_data);
		
		} else {
			fprintf(stderr, "Error reading input file. Line prefix incorrect.\n");
			exit(EXIT_FAILURE);
		}
	}
	fclose(file_in);	
	printf("Policy interpreter tool done!\n");
}

void assign_level_to_file(char * assignee, char * assignment_data, char * path_to_files) {
	errno = 0;
	size_t file_path_len = strlen(assignee) + strlen(path_to_files) + 1;
	// Create file path from path to dir and file name
	char * file_path = malloc(file_path_len * sizeof(char));
	strcpy(file_path, path_to_files);
	strcat(file_path, assignee);
	// Check if file exists
	FILE * file; 
	if(file = fopen(file_path, "r")) {
		fclose(file);
	} else {
		fprintf(stderr, "File %s does not exist\n", file_path);
		exit(EXIT_FAILURE);
	}

	// Assign level to file
	if(setxattr(file_path, "security.fsc.level", assignment_data, strlen(assignment_data), 0) == -1) {    
		setxattr_error_prints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", assignment_data, file_path, errno);
		exit(EXIT_FAILURE);
        }
	file_path = "";
}

void assign_label_to_file(char * assignee, char * assignment_data, char * path_to_files) {
	errno = 0;
	size_t file_path_len = strlen(assignee) + strlen(path_to_files) + 1;

	// Create file path from path to dir and file name
	char * file_path = malloc(file_path_len * sizeof(char));
	strcpy(file_path, path_to_files);
	strcat(file_path, assignee);

	// Reset label extended attributes
	if(!file_already_accessed(file_path)) {
		if(setxattr(file_path, "security.fsc.labels", "", 0, 0) == -1) {
			fprintf(stderr, "Error reseting labels for file %s - Errno: %d\n", file_path, errno);
			exit(EXIT_FAILURE);
		}
	}

	// Check if file exists
	FILE * file; 
	if(file = fopen(file_path, "r")) {
		fclose(file);
	} else {
		fprintf(stderr, "File %s does not exist.\n", file_path);
		exit(EXIT_FAILURE);
	}

	char * xattr = malloc(500);
	int xattr_size = getxattr(file_path, "security.fsc.labels", xattr, 500);
	if(xattr_size == -1) {
		getxattr_error_prints();
		fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", file_path, errno);
                exit(EXIT_FAILURE);
	}
	// If the file does not already have a label attribute, create it
	// else, add to existing file label attribute
	if(!xattr_size) {
		size_t data_len = strlen(assignment_data) + 1;
		char * data = malloc(data_len * sizeof(char));

		// Build JSON
		strcpy(data, assignment_data);

		if(setxattr(file_path, "security.fsc.labels", data, strlen(data), 0) == -1) {
			fprintf(stderr, "Error setting label attribute %s for file %s - Errno: %d\n", data, file_path, errno);
			exit(EXIT_FAILURE);
		}
		free(data);
	} else {
		if(!label_exists_in_xattrs(assignment_data, xattr)) {
			char * delimiter = ":";
			size_t data_len = strlen(assignment_data) + strlen(xattr) + strlen(delimiter) + 1;
			char * data = malloc(data_len * sizeof(char));

			// Build JSON
			strcpy(data, xattr);
			strcat(data, delimiter);
			strcat(data, assignment_data);
			if(setxattr(file_path, "security.fsc.labels", data, strlen(data), 0) == -1) {
				setxattr_error_prints();
				fprintf(stderr, "Error adding label attribute %s for file %s.\n", data, file_path);
				exit(EXIT_FAILURE);
			}
			free(data);
		}
	}
	file_path = "";
}

int label_exists_in_xattrs(char * check_label, char * file_json) {
	regex_t reegex;
	// Build JSON object to search for
	char * prefix = "({|:)";
	char * postfix = "(:|})";
	size_t data_len = strlen(check_label) + strlen(prefix) + strlen(postfix) + 1;
	char * check_label_with_delim = malloc(data_len * sizeof(char));

	strcpy(check_label_with_delim, prefix);
	strcat(check_label_with_delim, check_label);
	strcat(check_label_with_delim, postfix);
	int value;

	value = regcomp(&reegex, check_label_with_delim, 0);
	value = regexec(&reegex, file_json, 0, NULL, 0);

	return value == 0;
}
	
void write_user_level_to_db(char * assignee, char * assignment_data) {
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
	
	if(!db_accessed) {
		fclose(fopen("targeted_users_db.txt", "w"));
	}

	FILE * out_file = fopen("targeted_users_db.txt", "a");
 	if(out_file == NULL) {
                fprintf(stderr, "Error reading or creating output file.\n");
                exit(EXIT_FAILURE);
        }	

	if(!db_accessed)	{
		db_accessed = 1;
	}

	fprintf(out_file, "%s:%s\n", assignee, assignment_data);
	fclose(out_file);
}

void write_user_label_to_db(char * assignee, char * assignment_data) {
	FILE * out_file = fopen("targeted_users_db.txt", "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int found_user = 0;
	char * out_string = malloc(1024);

	if(out_file == NULL) {
		fprintf(stderr, "Output file does not exist for writing label.\n");
		exit(EXIT_FAILURE);
	}

	while((read = getline(&line, &len, out_file)) != -1) {
		char * token = strtok(strdup(line), ":");
		if(strcmp(token, assignee) == 0) {
			line[strcspn(line, "\n")] = 0;
			strcat(out_string, line);
			strcat(out_string, ":");
			strcat(out_string, assignment_data);
			strcat(out_string, "\n");
			found_user = 1;
		} else {
			strcat(out_string, line);
		}
	}
	fclose(out_file);

	out_file = fopen("targeted_users_db.txt", "w");
	fprintf(out_file, "%s", out_string);

	if(!found_user){
		fprintf(stderr, "Could not find user that label belongs to.\n");
		exit(EXIT_FAILURE);
	}
	free(out_string);
}

int file_already_accessed(char * file) {
	int index = 0;
	char * cur_file = NULL;
	if(file_paths[0]) {
		cur_file = strdup(file_paths[0]);
	}
	while(cur_file) {
		if(strcmp(cur_file, file) == 0) {
			// File found
			return 1;
		}
		index++;
		cur_file = file_paths[index];
	}

	file_paths = (char **)realloc(file_paths, (index + 1) * sizeof(char *));
	file_paths[index] = strdup(file);
	return 0;
}

void setxattr_error_prints() {
	printf("EDQUOT: %d\nEExist XATTR_CREATE: %d\nENODATA XATTR_REPLACE: %d\nENOSPC:%d\nENOTSUP: %d\nEPERM: %d\nERANGE: %d\n", EDQUOT, EEXIST, ENODATA, ENOSPC, ENOTSUP, EPERM, ERANGE);
}
	
void getxattr_error_prints() {
	printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}
