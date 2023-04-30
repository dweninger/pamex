/**
 * This is the fludb tool for PAMEx
 * This tool is used to interpret the file and user assignment file 
 * 	and create the targeted users database and file's extended attributes
 * Author: Daniel Weninger
 * Last Modified: 4/29/2023
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <sys/xattr.h>
#include <errno.h>
#include <regex.h>

#include "fludb.h"

int file_paths_size;
char ** file_paths;

int main(int ac, char ** av) {

	// Check if there are correct amount of params
	if(ac != 4) {
		fprintf(stderr, "usage: %s <policy_filepath> <dir_of_files> <user_db_out_path>\n", av[0]);
		exit(EXIT_FAILURE);
	}

	printf("Starting FLUDB tool...\n");

	// Initiate global variables
	file_paths_size = 0;
	file_paths = (char **)malloc(sizeof(char*));

	// Initiate local variables
	char * path_to_files = strdup(av[2]);
	char * user_db_path = strdup(av[3]);
	FILE * file_in = fopen(av[1], "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	// Check if file_in exists
	if(file_in == NULL) {
		fprintf(stderr, "Input file does not exist\n");
		exit(EXIT_FAILURE);
	}

	FILE * out_file = fopen(user_db_path, "w");
	if(out_file == NULL) {
		fprintf(stderr, "Error creating output file.\n");
		exit(EXIT_FAILURE);
	}	
	fclose(out_file);

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
			write_user_level_to_db(assignee, assignment_data, user_db_path);
		} else if (strcmp(assignment_type, "USER_LABELS") == 0) {
			write_user_label_to_db(assignee, assignment_data, user_db_path);
		
		} else {
			fprintf(stderr, "Error reading input file. Line prefix incorrect.\n");
			exit(EXIT_FAILURE);
		}
	}
	fclose(file_in);	
	printf("FLUDB tool done!\n");
}

/**
 * assign_level_to_file - write a file's level information to that file's level extended attribute
 * assignee - file path to assign the extended attribute to
 * assignment_data - the value of the extended attribute. name and placement of level
 * path_to_files - the path to the parent directory containing all of the files to assign
 * 	extended attributes to
*/
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
		// Assign level to file
		if(setxattr(file_path, "security.pamex.level", assignment_data, strlen(assignment_data), 0) == -1) {    
			fprintf(stderr, "Error setting level attribute %s for file %s - %s\n", assignment_data, file_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "File %s does not exist\n", file_path);
	}

	file_path = "";
}

/**
 * assign_label_to_file - add a file's label information to that file's label extended attribute
 * assignee - file path to assign the extended attribute to
 * assignment_data - the value of the extended attribute. name of label
 * path_to_files - the path to the parent directory containing all of the files to assign
 * 	extended attributes to
*/
void assign_label_to_file(char * assignee, char * assignment_data, char * path_to_files) {
	errno = 0;
	size_t file_path_len = strlen(assignee) + strlen(path_to_files) + 1;

	// Create file path from path to dir and file name
	char * file_path = malloc(file_path_len * sizeof(char));
	strcpy(file_path, path_to_files);
	strcat(file_path, assignee);

	// Reset label extended attributes
	if(!file_already_accessed(file_path)) {
		if(setxattr(file_path, "security.pamex.labels", "", 0, 0) == -1) {
			fprintf(stderr, "Error reseting labels for file %s - %s\n", file_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	// Check if file exists
	FILE * file; 
	if(file = fopen(file_path, "r")) {
		fclose(file);
		char * xattr = malloc(500);
	int xattr_size = getxattr(file_path, "security.pamex.labels", xattr, 500);
	if(xattr_size == -1) {
		fprintf(stderr, "Error getting label attributes for file %s - %s\n", file_path, strerror(errno));
        exit(EXIT_FAILURE);
	}
	// If the file does not already have a label attribute, create it
	// else, add to existing file label attribute
	if(!xattr_size) {
		size_t data_len = strlen(assignment_data) + 1;
		char * data = malloc(data_len * sizeof(char));

		// Build JSON
		strcpy(data, assignment_data);

		if(setxattr(file_path, "security.pamex.labels", data, strlen(data), 0) == -1) {
			fprintf(stderr, "Error setting label attribute %s for file %s - %s\n", data, file_path, strerror(errno));
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
			if(setxattr(file_path, "security.pamex.labels", data, strlen(data), 0) == -1) {
				fprintf(stderr, "Error adding label attribute %s for file %s.\n", data, file_path);
				exit(EXIT_FAILURE);
			}
			free(data);
		}
	}
	file_path = "";
	} else {
		fprintf(stderr, "File %s does not exist.\n", file_path);
	}
}

/**
 * label_exists_in_xattrs - check if a label is in the list of labels for the file's labels
 * 	extended attribute
 * check_label - name of the label to find in the extended attribute
 * file_json - the value of the extended attribute containing all of the file's labels
 * returns - 1 if found and 0 if not found
*/
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

/**
 * write_user_level_to_db - writes the level of a user to the users database by searching
 * 	through the database for the user and adding the level information to that line
 * assignee - the user to give the level to
 * assignment_data - the level and its placement to give to the user
 * user_db_out_path - where the user database lives on the system
*/
void write_user_level_to_db(char * assignee, char * assignment_data, char * user_db_out_path) {
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	// Check file exists
	FILE * out_file = fopen(user_db_out_path, "a");
 	if(out_file == NULL) {
		fprintf(stderr, "Error creating output file.\n");
		exit(EXIT_FAILURE);
	}
	// Write level infor to db
	fprintf(out_file, "%s:%s\n", assignee, assignment_data);
	fclose(out_file);
}

/**
 * write_user_label_to_db - appends a label of a user to the users database by searching
 * 	through the database for the user and adding the label to that line
 * assignee - the user to give the label to
 * assignment_data - the label name to give to the user
 * user_db_out_path - where the user database lives on the system
*/
void write_user_label_to_db(char *assignee, char *assignment_data, char *user_db_out_path) {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_num = 0;
    int found_line = 0;
    char *out_string = malloc(sizeof(char) * 1024);
    out_string[0] = '\0'; // initialize with null terminator
	// Check DB exists
    fp = fopen(user_db_out_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", user_db_out_path);
        exit(EXIT_FAILURE);
    }
	// Find user in db and build db line for user
    while ((read = getline(&line, &len, fp)) != -1) {
        line_num++;
        char *token = strtok(strdup(line), ":");
        if (strcmp(token, assignee) == 0) {
            line[strcspn(line, "\r\n")] = 0;
            strcat(out_string, line);
            strcat(out_string, ":");
            strcat(out_string, assignment_data);
            strcat(out_string, "\n");
            found_line = 1;
        } else {
            strcat(out_string, line);
        }
    }

    if (!found_line) {
        fprintf(stderr, "Error: could not find line starting with %s\n", assignee);
        exit(EXIT_FAILURE);
    }
	fflush(fp);
    fclose(fp);

    fp = fopen(user_db_out_path, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file %s\n", user_db_out_path);
        exit(EXIT_FAILURE);
    }
	// Write label info
    fprintf(fp, "%s", out_string);
	fflush(fp);
    fclose(fp);
    free(out_string);
}

/**
 * file_already_accessed - Check if a file on a system has already been accesed. If not, reset the labels and levels
 * file - the file path of the file to check
 * returns  - 1 if file has been accessed and 0 if not
*/
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
	// File not found
	file_paths = (char **)realloc(file_paths, (index + 1) * sizeof(char *));
	file_paths[index] = strdup(file);
	return 0;
}
