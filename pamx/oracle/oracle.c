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
	if(argc != 3) {
		fprintf(stderr, "usage: %s <dir_containing_sudo_proc> <path_to_level_db>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    // Make sure level database exists based on user args
    char * levelDBPath = strdup(argv[2]);
    FILE * levelDB = fopen(levelDBPath, "r");
    if(!levelDB) {
        fprintf(stderr, "Could not find level DB\n");
        exit(EXIT_FAILURE);
    }
    fclose(levelDB);

    printf("Welcome to the Pamx Oracle!\n");

    // Prompt user for PID
    // This is the name of the dir that the attr/current file is in
    printf("Enter PID\n");
    printf(" > ");
    scanf("%s", userCommand); 
	
	// Set proc path based on user input
	char * procFilePath = malloc(200 * sizeof(char));
    sprintf(procFilePath, "%s/sudo_proc/%s/attr/current", argv[1], userCommand);

    // Make sure proc file exists
    FILE * procFile = fopen(procFilePath, "r");
    // If the proc file was not found, prompt for it again
    while(!procFile) {
        printf("Proc file not found for pid %s. Try again.\n", userCommand);
        printf(" > ");
        scanf("%s", userCommand);
        sprintf(procFilePath, "%s/sudo_proc/%s/attr/current", argv[1], userCommand);
        procFile = fopen(procFilePath, "r");
    }
    // Check that the proc file contains information
    if(read = getline(&line, &len, procFile) == -1) {
        fprintf(stderr, "The proc file is empty\n");
        exit(EXIT_FAILURE);
    }
    fclose(procFile);
    
    printf("Pid accepted.\n");

    // Keep prompting user for commands until they quit
    printf("Type help for a list of commands.\n");
    printf(" > ");
    scanf("%s", userCommand); 
    while(strcasecmp(userCommand, "quit") != 0) {
        
        if(strcasecmp(userCommand, "help") == 0) {
            // help
            printf("Pamx Oracle Commands:\n"
            "help - print list of commands\n"
            "user - check the name of the signed in user\n"
            "userinfo - get the name and authentication information of signed in user\n"
            "checkfileaccess - check if the currently signed in user can access a file\n"
            "cfa - alias for the checkfileaccess command\n"
            "fileinfo - get the authentication information of a file\n"
            "quit - quit the Oracle\n");
        
        } else if(strcasecmp(userCommand, "user") == 0) {
            // check username
            procFile = fopen(procFilePath, "r");
            read = getline(&line, &len, procFile);
            char * token = strtok(strdup(line), ":");
            printf("%s\n", token);
            free(token);
            fclose(procFile);
        
        } else if (strcasecmp(userCommand, "userinfo") == 0) {
            // check user info
            procFile = fopen(procFilePath, "r");
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
            fclose(procFile);
        
        } else if (strcasecmp(userCommand, "checkfileaccess") == 0 || strcasecmp(userCommand, "cfa") == 0) {
            // check if a user can access a file
            int userlevel = getUserLevel(procFilePath, levelDBPath);
            char ** userlabels = getUserLabels(procFilePath);
            printf("File path: ");
            char targetedFilePath[100];
            scanf("%s", targetedFilePath);

            FILE * targetedFile = fopen(targetedFilePath, "r");
            if(!targetedFile) {
                fprintf(stderr, "File not found at %s\n", targetedFilePath);
                goto LOOP;
            }
            fclose(targetedFile);

            int filelevel = getFileLevel(targetedFilePath, levelDBPath);
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
        
        } else if (strcasecmp(userCommand, "fileinfo") == 0) {
            // check file info
            printf("File path: ");
            char targetedFilePath[100];
            scanf("%s", targetedFilePath);

            FILE * targetedFile = fopen(targetedFilePath, "r");
            if(!targetedFile) {
                fprintf(stderr, "File not found at %s\n", targetedFilePath);
                goto LOOP;
            }
            fclose(targetedFile);

            char * levelname = "unclassified";
            int levelplacement = 0;
            char * levelxattr = malloc(500);
            int xattrSize = getxattr(targetedFilePath, "security.fsc.level", levelxattr, 500);
            if(xattrSize != -1) {
                char * token = strtok(levelxattr, ":");
                levelname = strdup(token);
	            token = strtok(NULL, ":");
                levelplacement = getFileLevel(targetedFilePath, levelDBPath);
            }

            printf("{\"file\" : \"%s\", \"level\" : \"%s\", \"placement\" : \"%d\", \"labels\" : [", targetedFilePath, levelname, levelplacement);
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

    LOOP:
        printf(" > ");
        scanf("%s", userCommand); 
    }

}

/**
 * getUserLevel - get the hierarchical level of the user from the sudo proc file
 * targetedUsersDBFile - the filepointer of the targeted users DB 
 */
int getUserLevel(char * procFilePath, char * levelDBPath) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * levelName = malloc(200);
    int placement = -1;
    FILE * procFile = fopen(procFilePath, "r");
    FILE * levelDBFile = fopen(levelDBPath, "r");

	read = getline(&line, &len, procFile);
    char * token = strtok(strdup(line), ":");
    token = strtok(NULL, ":");
    strcpy(levelName, token);

    while ((read = getline(&line, &len, levelDBFile)) != -1) {
        char * levelToken = strtok(strdup(line), ":");
        if(strcmp(levelName, levelToken) == 0) {
            levelToken = strtok(NULL, ":");
            placement = atoi(levelToken);
        }
    }

    if(placement == -1) {
        fprintf(stderr, "Could not find user's level %s in the level DB\n", levelName);
        exit(EXIT_FAILURE);
    }

    fclose(procFile);
    fclose(levelDBFile);
	return placement;
}

char ** getUserLabels(char * procFilePath) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
	char ** labelList = (char**)malloc(sizeof(char*));
	int index = 0;
    FILE * procFile = fopen(procFilePath, "r");

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

    fclose(procFile);
    return labelList;
}

int getFileLevel(char * targetedFilePath, char * levelDBPath) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * levelName = malloc(200);
    int placement = -1;
	char * xattr = malloc(500);

    int xattrSize = getxattr(targetedFilePath, "security.fsc.level", xattr, 500);
    if(xattrSize == -1) {
        return 0;
    }

	char * token = strtok(xattr, ":");
    strcpy(levelName, token);
    FILE * levelDBFile = fopen(levelDBPath, "r");
    while ((read = getline(&line, &len, levelDBFile)) != -1) {
        char * levelToken = strtok(strdup(line), ":");
        if(strcmp(levelName, levelToken) == 0) {
            levelToken = strtok(NULL, ":");
            placement = atoi(levelToken);
        }
    }
    
    if(placement == -1) {
        fprintf(stderr, "Could not find user's level %s in the level DB\n", levelName);
        exit(EXIT_FAILURE);
    }

    fclose(levelDBFile);
	return placement;
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
