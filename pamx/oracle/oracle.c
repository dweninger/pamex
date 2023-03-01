/**
 * This is the Oracle tool for Pamx
 * The job of this tool is to simulate how a custom kernel would cross reference
 *  the logged in user's credentials via the proc directory and the exteneded
 *  attributes to ensure that a user is allowed to access the file. The Oracle
 *  tool simulates this by checking a custom-made proc-like file and the file's
 *  extended attributes.
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

#include <sys/xattr.h>
#include <errno.h>
#include <string.h>
#include <strings.h>

#include "oracle.h"

int main (int argc, char ** argv) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char userCommand[100];

    // Check args
	if(argc > 1) {
		fprintf(stderr, "usage: %s\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	printf("Welcome to the Pamx Oracle!\n");
    printf("Enter PID\n");
    printf(" > ");
    scanf("%s", userCommand); 
	
	// Set proc path
	char * procFilePath = malloc(200 * sizeof(char));
    sprintf(procFilePath, "../sudo_proc/%s/attr/current", userCommand);

    FILE * procFile = fopen(procFilePath, "r");
    while(!procFile) {
        printf("Proc file not found for pid %s\n", userCommand);
        printf(" > ");
        scanf("%s", userCommand);
    }
    
    if(read = getline(&line, &len, procFile) == -1) {
        fprintf(stderr, "The proc file is empty\n");
        exit(EXIT_FAILURE);
    }
    printf("Pid accepted.\n");
    printf("Type help for a list of commands.\n");
    printf(" > ");
    scanf("%s", userCommand); 
    
    while(strcasecmp(userCommand, "quit") != 0){
        fseek(procFile, 0, SEEK_SET);
        // help
        if(strcasecmp(userCommand, "help") == 0) {
            printf("Pamx Oracle Commands:\n"
            "help - print list of commands\n"
            "user - check the name of the signed in user\n"
            "userinfo - get the name and authentication information of signed in user\n"
            "checkfileaccess - check if the currently signed in user can access a file\n"
            "cfa - alias for the checkfileaccess command\n"
            "fileinfo - get the authentication information of a file\n"
            "quit - quit the Oracle\n");
        // check username
        } else if(strcasecmp(userCommand, "user") == 0) {
            read = getline(&line, &len, procFile);
            char * token = strtok(strdup(line), ":");
            printf("%s\n", token);
            free(token);
        // check user info
        } else if (strcasecmp(userCommand, "userinfo") == 0) {
            read = getline(&line, &len, procFile);
            char * token = strtok(strdup(line), ":");
            printf("{\"username\" : \"%s\", ", token);
            token = strtok(NULL, ":");
            printf("\"level\" : \"%s\",", token);
            token = strtok(NULL, ":");
            printf("\"placement\" : \"%s\", ", token);
            printf("\"labels\" : [");
            token = strtok(NULL, ":");
            while(token) {
                token[strcspn(token, "\n")] = 0;
                printf("{\"name\" : \"%s\"}, ", token);
                token = strtok(NULL, ":");
            }
            printf("]}\n");
            free(token);
        // check if a user can access a file
        } else if (strcasecmp(userCommand, "checkfileaccess") == 0 || strcasecmp(userCommand, "cfa") == 0) {
            int userlevel = getUserLevel(procFile);
            fseek(procFile, 0, SEEK_SET);
            char ** userlabels = getUserLabels(procFile);
            printf("File path: ");
            char targetedFilePath[100];
            scanf("%s", targetedFilePath);
            FILE * targetedFile = fopen(targetedFilePath, "r");
            if(!targetedFile) {
                fprintf(stderr, "File not found at %s\n", targetedFilePath);
                break;
            }
            int filelevel = getFileLevel(targetedFilePath);
	        char ** filelabels = getFileLabels(targetedFilePath);

            if(userlevel >= filelevel) {
                if(containsLabels(filelabels, userlabels)) {
                    printf("\nAccess granted!\n");
                } else {
                    printf("\nAccess denied. User does not have appropriate labels.\n");
                }
            } else {
                printf("\nAccess denied. User level too low.\n");
            }
        // check file info
        } else if (strcasecmp(userCommand, "fileinfo") == 0) {
            printf("File path: ");
            char targetedFilePath[100];
            scanf("%s", targetedFilePath);

            FILE * targetedFile = fopen(targetedFilePath, "r");
            if(!targetedFile) {
                fprintf(stderr, "File not found at %s\n", targetedFilePath);
                break;
            }
            char * levelname = "unclassified";
            char * levelplacement = "0";
            char * levelxattr = malloc(500);
            int xattrSize = getxattr(targetedFilePath, "security.fsc.level", levelxattr, 500);
            if(xattrSize != -1) {
                char * token = strtok(levelxattr, ":");
                levelname = strdup(token);
	            token = strtok(NULL, ":");
                levelplacement = strdup(token);
            }

            printf("{\"file\" : \"%s\", \"level\" : \"%s\", \"placement\" : \"%s\", \"labels\" : [", targetedFilePath, levelname, levelplacement);
            char * labelxattr = malloc(500);
            xattrSize = getxattr(targetedFilePath, "security.fsc.labels", labelxattr, 500);
            if(xattrSize != -1) {
               char * token = strtok(labelxattr, ":");
                while(token) {
                    printf("{\"name\" : \"%s\"}, ", token);
                    token = strtok(NULL, ":");
                } 
            }
            printf("]}\n");
        } else {
            printf("Not a valid command. Type help for a list of commands.\n");
        }
        printf(" > ");
        scanf("%s", userCommand); 
    }

	fclose(procFile);
}

/**
 * getUserLevel - get the hierarchical level of the user from the sudo proc file
 * targetedUsersDBFile - the filepointer of the targeted users DB 
 */
int getUserLevel(FILE * procFile) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
	read = getline(&line, &len, procFile);
    char * token = strtok(strdup(line), ":");
    token = strtok(NULL, ":");
    token = strtok(NULL, ":");
	return atoi(token);
}

char ** getUserLabels(FILE * procFile) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
	char ** labelList = (char**)malloc(sizeof(char*));
	int index = 0;
    read = getline(&line, &len, procFile);
    char * token = strtok(strdup(line), ":");
    token = strtok(NULL, ":");
    token = strtok(NULL, ":");
	token = strtok(NULL, ":");

    while(token) {
        labelList = (char**)realloc(labelList, (index + 1) * sizeof(char *));
        labelList[index] = strdup(token);
        index++;
        token = strtok(NULL, ":");
    }

    return labelList;
}

int getFileLevel(char * targetedFilePath) {
	char * xattr = malloc(500);
    int xattrSize = getxattr(targetedFilePath, "security.fsc.level", xattr, 500);
    if(xattrSize == -1) {
        return 0;
    }

	char * token = strtok(xattr, ":");
	token = strtok(NULL, ":");
	return atoi(token);
}

char ** getFileLabels(char * targetedFilePath) {
	char * xattr = malloc(500);
	char ** labelList = (char**)malloc(sizeof(char*));
	int index = 0;

	int xattrSize = getxattr(targetedFilePath, "security.fsc.labels", xattr, 500);
	if(xattrSize == -1) {
		if(errno == ENODATA) {
			return NULL;
		} else {
			getxattrErrorPrints();
			fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", targetedFilePath, errno);
			exit(EXIT_FAILURE);
		}
	}
	
	char * token = strtok(xattr, ":");
	while(token) {
		labelList = (char **)realloc(labelList, (index +1) * sizeof(char*));
		labelList[index] = strdup(token);
		token = strtok(NULL, ":");
		index++;
	}
	return labelList;
}

int containsLabels(char ** refLabels, char ** userLabels) {
	int refI = 0;
	int userI = 0;
	int foundMatch = 0;
	while(refLabels && refLabels[refI] && strcmp(refLabels[refI], "") != 0) { 
		userI = 0;
		while(userLabels && userLabels[userI] && strcmp(userLabels[userI], "") != 0) {
            userLabels[userI][strcspn(userLabels[userI], "\n")] = 0;
			if(strcmp(refLabels[refI], userLabels[userI]) == 0) {
				foundMatch = 1;
				break;
			}			
			userI++;	
		}
		if(!foundMatch) {
			return 0;
		}
		foundMatch = 0;
		refI++;
	}
	return 1;
}

void getxattrErrorPrints() {
        printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}
