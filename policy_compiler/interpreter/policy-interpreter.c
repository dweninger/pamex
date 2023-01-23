#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <sys/xattr.h>
#include <errno.h>
#include <regex.h>

#include "policy-interpreter.h"

int filePathsSize;
char ** filePaths;

int main(int ac, char ** av) {
	// Check if there are correct amount of params
	if(ac != 3) {
		fprintf(stderr, "usage: %s policy_filepath dir_of_files\n", av[0]);
		exit(EXIT_FAILURE);
	}
	filePathsSize = 0;
	filePaths = (char **)malloc(sizeof(char*));
	// Create vars
	char * pathToFiles = strdup(av[2]);
	FILE * filein = fopen(av[1], "r");
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	// Check if filein exists
	if(filein == NULL) {
		fprintf(stderr, "Input file does not exist\n");
		exit(EXIT_FAILURE);
	}

	// Read filein line by line	
	while((read = getline(&line, &len, filein)) != -1) {
		// read line data and separate into vars based on space delimiter
		char * assignmentType;
		char * assignee;
		char * assignmentData;
         	char * token = strtok(line, " ");
         	assignmentType = strdup(token);
		token = strtok(NULL, " ");
		assignee = strdup(token);
		token = strtok(NULL, " ");
		assignmentData = strdup(token);
		// Remove trailing newline
		assignmentData[strcspn(assignmentData, "\r\n")] = 0;	
		// Check which operation to do based on line prefix	
		if (strcmp(assignmentType, "FILE_LEVEL") == 0) {
			assignLevelToFile(assignee, assignmentData, pathToFiles);			
		
		} else if (strcmp(assignmentType, "FILE_LABELS") == 0) {
			assignLabelToFile(assignee, assignmentData, pathToFiles);
		
		} else if (strcmp(assignmentType, "USER_LEVEL") == 0) {
			assignLevelToUser(assignee, assignmentData);
		
		} else if (strcmp(assignmentType, "USER_LABELS") == 0) {
			collectLabelForUser(assignee, assignmentData);
		
		} else {
			fprintf(stderr, "Error reading input file. Line prefix incorrect.\n");
			exit(EXIT_FAILURE);
		}
		
	}
	fclose(filein);	
	printf("Policy tool worked.\n");
}

void assignLevelToFile(char * assignee, char * assignmentData, char * pathToFiles) {
	// Create file path from path to dir and file name
	char * filePath = malloc(500);
	strcat(filePath, pathToFiles);
	strcat(filePath, assignee);
	errno = 0;	
	// Check if file exists
	FILE * file; 
	if(file = fopen(filePath, "r")) {
		fclose(file);
	} else {
		fprintf(stderr, "File %s does not exist\n", filePath);
		exit(EXIT_FAILURE);
	}

	// Assign level to file
	if(setxattr(filePath, "security.fsc.level", assignmentData, strlen(assignmentData), 0) == -1) {    
        	fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", assignmentData, filePath, errno);
		exit(EXIT_FAILURE);
        }
	filePath = "";
}


void assignLabelToFile(char * assignee, char * assignmentData, char * pathToFiles) {
	errno = 0;
	// Create file path from path to dir and file name
	char * filePath = malloc(500);
	strcat(filePath, pathToFiles);
	strcat(filePath, assignee);
	
	// Reset label extended attributes
	if(!fileAlreadyAccessed(filePath)) {
		if(setxattr(filePath, "security.fsc.labels", "", 0, 0) == -1) {
			fprintf(stderr, "Error reseting labels for file %s - Errno: %d\n", filePath, errno);
			exit(EXIT_FAILURE);
		}
	}

	// Check if file exists
	FILE * file; 
	if(file = fopen(filePath, "r")) {
		fclose(file);
	} else {
		fprintf(stderr, "File %s does not exist.\n", filePath);
		exit(EXIT_FAILURE);
	}

	char * xattr = malloc(sizeof(char *));
	int xattrSize = getxattr(filePath, "security.fsc.labels", xattr, sizeof(char *));

	// If the file does not already have a label attribute, create it
	// else, add to existing file label attribute
	if(!xattrSize) {
		char * data = malloc(500);
		// Build JSON
		strcat(data, "{\"labels\":[{\"name\":\"");
		strcat(data, assignmentData);
		strcat(data, "\"}]}");
		
		if(setxattr(filePath, "security.fsc.labels", data, strlen(data), 0) == -1) {
			fprintf(stderr, "Error setting label attribute %s for file %s - Errno: %d\n", data, filePath, errno);
			exit(EXIT_FAILURE);
		}
		free(data);
	} else {
		if(!labelExistsInFile(assignmentData, xattr)) {
			char * data = malloc(500);
			// Build JSON
			char * token = strtok(xattr, "]}");
			strcat(data, token);
			strcat(data, ",{\"name\":\"");
			strcat(data, assignmentData);
			strcat(data, "\"}]}");
		
			if(setxattr(filePath, "security.fsc.labels", data, strlen(data), 0) == -1) {
				fprintf(stderr, "Error adding label attribute %s for file %s.\n", data, filePath);
				exit(EXIT_FAILURE);
			}
			free(data);
		}
	}
	filePath = "";
}

int labelExistsInFile(char * checkLabel, char * fileJson) {
	regex_t reegex;
	// Build JSON object to search for
	char * checkJson = malloc(500);
	strcat(checkJson, "{\"name\":\"");
	strcat(checkJson, checkLabel);
	strcat(checkJson, "\"}");
	int value;

	value = regcomp(&reegex, checkJson, 0);
	value = regexec(&reegex, fileJson, 0, NULL, 0);

	return value == 0;
}

	
void assignLevelToUser(char * assignee, char * assignmentData) {
	
}

void collectLabelForUser(char * assignee, char * assignmentData) {

}

int fileAlreadyAccessed(char * file) {
	int index = 0;
	char * curFile = filePaths[0];

	while(curFile){
		if(strcmp(curFile, file) == 0) {
			// File found
			return 1;
		}
		index++;
		curFile = filePaths[index];
	}

	filePaths = (char **)realloc(filePaths, (index + 1) * sizeof(char *));
	filePaths[index] = strdup(file);
	return 0;
}
