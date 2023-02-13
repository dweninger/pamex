#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdlib.h>
#include <sys/xattr.h>
#include <errno.h>

static struct pam_conv conv = {
	misc_conv,
	NULL
};

int getUserLevel(char * username, FILE * targetedUsersDBFile);
char ** getUserLabels(char * username, FILE * targetedUsersDBFile);
int getFileLevel(char * targetedFilePath);
char ** getFileLabels(char * targetedFilePath);
int containsLabels(char ** refLabels, char ** userLabels);
void getxattrErrorPrints();

int main (int argc, char ** argv) {
	printf("Starting pam module...\n");
	// Check args
	if(argc < 2 || argc > 3) {
		fprintf(stderr, "usage: %s <targeted_file_path> [targeted_users_db_path]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// Set targeted users DB path
	char * targetedUsersDBPath = "./targeted_users_db.txt";
	if(argc == 3) {
		targetedUsersDBPath = strdup(argv[2]);
	}

	char * targetedFilePath = strdup(argv[1]);

	FILE * targetedUsersDBFile = fopen(targetedUsersDBPath, "r");
	FILE * targetedFile = fopen(targetedFilePath, "r");

	pam_handle_t *handle = NULL;
	const char *service_name = "../../../../../ect/pam.d/pam_example";
	int retval;
	char *username; // To be set by pam_get_item
	// PAM authenticate
	retval = pam_start(service_name, NULL, &conv, &handle);
	if(retval != PAM_SUCCESS) {
		fprintf(stderr, "Failure in pam initialization: %s\n", pam_strerror(handle, retval));
		return 1;
	}
	
	retval = pam_authenticate(handle, 0); // prompt user for username and pasword
	if(retval != PAM_SUCCESS) {
		fprintf(stderr, "Failure in pam authentication: %s\n", pam_strerror(handle, retval));
		return 1;
	}

	retval = pam_acct_mgmt(handle, 0); // Check that account can access the system
	if(retval != PAM_SUCCESS) {
		fprintf(stderr, "Failure in pam account management: %s\n", pam_strerror(handle, retval));
		return 1;
	}
	
	pam_get_item(handle, PAM_USER, (const void **)&username);
	printf("WELCOME, %s\n", username);

	int userlevel = getUserLevel(username, targetedUsersDBFile);
	if(userlevel == -1){
		fprintf(stderr, "User not found in user DB\n");
		exit(EXIT_FAILURE);
	}
	char ** userlabels = getUserLabels(username, targetedUsersDBFile);

	int filelevel = getFileLevel(targetedFilePath);
	char ** filelabels = getFileLabels(targetedFilePath);

	if(userlevel >= filelevel && containsLabels(filelabels, userlabels)) {
		// grant access
		printf("Access granted!\n");
	} else {
		// deny access
		printf("Access denied.\n");
	}
	fclose(targetedUsersDBFile);
	fclose(targetedFile);
	pam_end(handle, retval); // Terminate PAM transaction
}

/**
 * getUserLevel - get the hierarchical level of the user from the targeted
 * 	users database
 * username  - the username of the user that is signed in
 * targetedUsersDBFile - the filepointer of the targeted users DB 
 */
int getUserLevel(char * username, FILE * targetedUsersDBFile) {
	char * line = NULL;
        size_t len = 0;
        ssize_t read;
	while((read = getline(&line, &len, targetedUsersDBFile)) != -1) {
                char * token = strtok(strdup(line), ":");
		if(strcmp(token, username) == 0) {
			token = strtok(NULL, ":");
			token = strtok(NULL, ":");
			return atoi(token);
		}	
	}
	return -1;
}

char ** getUserLabels(char * username, FILE * targetedUsersDBFile) {
	char * line = NULL;
        size_t len = 0;
        ssize_t read;
	char ** labelList = (char**)malloc(sizeof(char*));
	int index = 0;
        while((read = getline(&line, &len, targetedUsersDBFile)) != -1) {
        	char * token = strtok(strdup(line), ":");
                if(strcmp(token, username) == 0) {
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
        }
        return labelList;
}

int getFileLevel(char * targetedFilePath) {
	char * xattr = malloc(500);
        int xattrSize = getxattr(targetedFilePath, "security.fsc.level", xattr, 500);
        if(xattrSize == -1) {
                fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", targetedFilePath, errno);
                exit(EXIT_FAILURE);
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
	printf("containsLabels\n");
	int refI = 0;
	int userI = 0;
	int foundMatch = 0;
	while(refLabels && refLabels[refI] && strcmp(refLabels[refI], "") != 0) { 
		printf("label: %s\n", refLabels[refI]);
		userI = 0;
		while(userLabels && userLabels[userI] && strcmp(userLabels[userI], "") != 0) {
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
