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
	printf("Starting policy interpreter tool...\n");

	// Check if there are correct amount of params
	if(ac != 3) {
		fprintf(stderr, "usage: %s <policy_filepath> <dir_of_files>\n", av[0]);
		exit(EXIT_FAILURE);
	}

	// Initiate global variables
	filePathsSize = 0;
	filePaths = (char **)malloc(sizeof(char*));

	// Initiate local variables
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
	printf("Policy interpreter tool done!\n");
}

void assignLevelToFile(char * assignee, char * assignmentData, char * pathToFiles) {
	errno = 0;
	size_t filePathLen = strlen(assignee) + strlen(pathToFiles) + 1;
	// Create file path from path to dir and file name
	char * filePath = malloc(filePathLen * sizeof(char));
	strcpy(filePath, pathToFiles);
	strcat(filePath, assignee);
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
		setxattrErrorPrints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", assignmentData, filePath, errno);
		exit(EXIT_FAILURE);
        }
	filePath = "";
}

void assignLabelToFile(char * assignee, char * assignmentData, char * pathToFiles) {
	errno = 0;
	size_t filePathLen = strlen(assignee) + strlen(pathToFiles) + 1;

	// Create file path from path to dir and file name
	char * filePath = malloc(filePathLen * sizeof(char));
	strcpy(filePath, pathToFiles);
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

	char * xattr = malloc(500);
	int xattrSize = getxattr(filePath, "security.fsc.labels", xattr, 500);
	if(xattrSize == -1) {
		getxattrErrorPrints();
		fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", filePath, errno);
                exit(EXIT_FAILURE);
	}
	// If the file does not already have a label attribute, create it
	// else, add to existing file label attribute
	if(!xattrSize) {
		//char * jsonPrefix = "{\"labels\":[{\"name\":\"";
		//char * jsonPostfix = "\"}]}";
		//size_t dataLen = strlen(jsonPrefix) + strlen(jsonPostfix) + strlen(assignmentData) + 1;
		size_t dataLen = strlen(assignmentData) + 1;
		char * data = malloc(dataLen * sizeof(char));

		// Build JSON
		//strcpy(data, jsonPrefix);
		//strcat(data, assignmentData);
		//strcat(data, jsonPostfix);
		strcpy(data, assignmentData);

		if(setxattr(filePath, "security.fsc.labels", data, strlen(data), 0) == -1) {
			fprintf(stderr, "Error setting label attribute %s for file %s - Errno: %d\n", data, filePath, errno);
			exit(EXIT_FAILURE);
		}
		free(data);
	} else {
		if(!labelExistsInFile(assignmentData, xattr)) {
			//char * jsonPrefix = ",{\"name\":\"";
			//char * jsonPostfix = "\"}]}";
			//char * token = strtok(xattr, "]}");
			char * delimiter = ":";
			//size_t dataLen = strlen(jsonPrefix) + strlen(jsonPostfix) + strlen(assignmentData) + strlen(token) + 1;
			size_t dataLen = strlen(assignmentData) + strlen(xattr) + strlen(delimiter) + 1;
			char * data = malloc(dataLen * sizeof(char));

			// Build JSON
			//strcpy(data, token);
			//strcat(data, jsonPrefix);
			//strcat(data, assignmentData);
			//strcat(data, jsonPostfix);
			strcpy(data, xattr);
			strcat(data, delimiter);
			strcat(data, assignmentData);
			if(setxattr(filePath, "security.fsc.labels", data, strlen(data), 0) == -1) {
				setxattrErrorPrints();
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
	//char * jsonPrefix = "{\\\"name\\\":\\\"";
	//char * jsonPostfix = "\\\"}";
	//size_t dataLen = strlen(jsonPrefix) + strlen(jsonPostfix) + strlen(checkLabel) + 1;
	char * prefix = "({|:)";
	char * postfix = "(:|})";
	size_t dataLen = strlen(checkLabel) + strlen(prefix) + strlen(postfix) + 1;
	char * checklabelwithdelim = malloc(dataLen * sizeof(char));
	//strcpy(checkJson, jsonPrefix);
	//strcat(checkJson, checkLabel);
	//strcat(checkJson, jsonPostfix);
	strcpy(checklabelwithdelim, prefix);
	strcat(checklabelwithdelim, checkLabel);
	strcat(checklabelwithdelim, postfix);
	int value;

	value = regcomp(&reegex, checklabelwithdelim, 0);
	value = regexec(&reegex, fileJson, 0, NULL, 0);

	return value == 0;
}
	
void assignLevelToUser(char * assignee, char * assignmentData) {
	
}

void collectLabelForUser(char * assignee, char * assignmentData) {

}

int fileAlreadyAccessed(char * file) {
	int index = 0;
	char * curFile = NULL;
	if(filePaths[0]) {
		curFile = strdup(filePaths[0]);
	}
	while(curFile) {
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

void setxattrErrorPrints() {
	printf("EDQUOT: %d\nEExist XATTR_CREATE: %d\nENODATA XATTR_REPLACE: %d\nENOSPC:%d\nENOTSUP: %d\nEPERM: %d\nERANGE: %d\n", EDQUOT, EEXIST, ENODATA, ENOSPC, ENOTSUP, EPERM, ERANGE);
}
	
void getxattrErrorPrints() {
	printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}