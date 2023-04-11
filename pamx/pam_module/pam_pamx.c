#include <security/pam_modules.h>
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

int createprocfile(int cpid, char * filepath, char * userInfo);
int makedir(char * path);
char * getUserFromDB(char * username, FILE * targetedUsersDBFile);
void getxattrErrorPrints();

int createprocfile(int cpid, char * filepath, char * userInfo) {
	strcat(filepath, "/sudo_proc");
	if (!makedir(filepath)) {
        	return 0;
    	}

	char * tempfile = malloc(sizeof(char) * 200);
	sprintf(tempfile, "%s/%d", filepath, cpid);
	sprintf(filepath, "%s", tempfile);
	if (!makedir(filepath)) {
        	return 0;
    	}

	sprintf(tempfile, "%s/attr", filepath);
	sprintf(filepath, "%s", tempfile);
	if (!makedir(filepath)) {
        	return 0;
    	}

	sprintf(tempfile, "%s/current", filepath);
	sprintf(filepath, "%s", tempfile);
	FILE * procFP = fopen(filepath, "w");
	fprintf(procFP, "%s", userInfo);
	fclose(procFP);
	return 1;
}

int makedir(char * path) {
	struct stat st = {0};
	if (stat(path, &st) == -1) {
		if (mkdir(path, 0777) == -1) {
			fprintf(stderr, "Error creating directory: %s\n", path);
			return 0;
		}
	}
	return 1;
}

/**
 * getUserLevel - get the hierarchical level of the user from the targeted
 * users database
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
	return "";
}

void getxattrErrorPrints() {
        printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	char *username = NULL;
	int cpid = getpid();
	char * filepath = NULL;
	int ret = 0;	
	
	// Get the username
	if (pam_get_user(pamh, (const char **)&username, NULL) != PAM_SUCCESS || username == NULL) {
		// Could not find user
		fprintf(stderr, "Could not find username.\n");
		return PAM_AUTH_ERR;
	}

	if(argc != 2) {
		printf("usage: <path_to_users_db> <dest_dir>");
		return PAM_IGNORE;
	}
	char * targetedUsersDBPath = strdup(argv[0]);

	FILE * targetedUsersDBFile = fopen(targetedUsersDBPath, "r");
	if(!targetedUsersDBFile) {
		fprintf(stderr, "No targeted users DB file at %s\n", targetedUsersDBPath);
		return PAM_IGNORE;
	}

	char * userInfo = strdup(getUserFromDB(username, targetedUsersDBFile));
	fclose(targetedUsersDBFile);
	if(strcmp(userInfo, "") == 0){
		// If the user is not found in the user DB
		// give them level unrestricted
		fprintf(stderr, "User not found in user DB\n");
		char * userInfoTemp = malloc(500);
		sprintf(userInfoTemp, "%s:unrestricted:0", username);
		userInfo = strdup(userInfoTemp);
	}
	filepath = strdup(argv[1]);
	ret = createprocfile(cpid, filepath, userInfo);
	if(ret == 0) {
		fprintf(stderr, "Cannot create file at specified location.\n");
		return PAM_IGNORE;
	}
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

