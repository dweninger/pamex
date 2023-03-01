/**
 * This is the PAM simulator for Pamx
 * This simulator observes a user login and creates a PAM-like
 * 	file of the user
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <sys/xattr.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>

#include "pam.h"

static struct pam_conv conv = {
	misc_conv,
	NULL
};

int main (int argc, char ** argv) {
	pam_handle_t *handle = NULL;
	const char *service_name = "../../../../../ect/pam.d/pam_example";
	int retval;
	char *username; // To be set by pam_get_item
	// Set targeted users DB path
	char * targetedUsersDBPath = "../pamx/data/targeted_users_db.txt";
	int cpid = getpid();
	char * filepath = malloc(200 * sizeof(char));
	sprintf(filepath, "../../../../../proc/%d/attr/current", cpid);

	printf("Starting pam module...\n");
	// Check args
	if(argc > 2) {
		fprintf(stderr, "usage: %s [targeted_users_db_path]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

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

	if(argc == 2) {
		targetedUsersDBPath = strdup(argv[1]);
	}

	FILE * targetedUsersDBFile = fopen(targetedUsersDBPath, "r");
	if(!targetedUsersDBFile) {
		fprintf(stderr, "No targeted users DB file at %s\n", targetedUsersDBPath);
		exit(EXIT_FAILURE);
	}

	char * userInfo = strdup(getUserFromDB(username, targetedUsersDBFile));
	fclose(targetedUsersDBFile);

	if(userInfo == NULL){
		// If the user is not found in the user DB
		// 	give them level unrestricted
		fprintf(stderr, "User not found in user DB\n");
		char * userInfoTemp = malloc(500);
		sprintf(userInfoTemp, "%s:unrestricted:0");
		userInfo = strdup(userInfoTemp);
		free(userInfoTemp);
	}
	printf("filepath: %s\n",filepath);
	FILE * fp = fopen(filepath, "w");
	fprintf(fp, "%s", userInfo);
	fclose(fp);
	printf ("child_PID = %d,parent_PID = %d\n", getpid(), getppid( ) );

	printf("You are now logged in to the kernel.\nEnter q to quit\n");

	pid_t pid = fork();
	if(pid == -1) {
		fprintf(stderr, "Failed forking child\n");
		exit(EXIT_FAILURE);
	} else if(pid == 0) {
		char user_command = '\0';
		scanf("%c", &user_command);
		while(user_command != 'q' && user_command !='Q') {
			scanf("%c", &user_command);
		}
		exit(0);
	} else {
		// waiting for child to terminate
		wait(NULL);
	}

	pam_end(handle, retval); // Terminate PAM transaction
	free(filepath);
}

/**
 * getUserLevel - get the hierarchical level of the user from the targeted
 * 	users database
 * username  - the username of the user that is signed in
 * targetedUsersDBFile - the filepointer of the targeted users DB 
 */
char * getUserFromDB(char * username, FILE * targetedUsersDBFile) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
	while((read = getline(&line, &len, targetedUsersDBFile)) != -1) {
        char * token = strtok(strdup(line), ":");
		if(strcmp(token, username) == 0) {
			free(token);
			return strdup(line);
		}	
	}
	return NULL;
}

void getxattrErrorPrints() {
        printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}
