/**
 * PAM Module for creating a pseudo-proc file when the user signs in.
 * 	The pseudo-proc file contains user information from the userdb
 * 
 * Author: Daniel Weninger
 * Last Modified: 3/27/2023
*/

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

void create_proc_file(int cpid, char * file_path, char * user_info);
void makedir(char * path);
char * get_user_from_db(char * username, FILE * targeted_users_db_file);

/**
 * create_proc_file - creates the directories for the pseudo-proc file
 * 	and writes to file
 * cpid - the process ID
 * file_path - the path to pseudo_proc directory
 * user_info - the pamx information from the userdb
*/
void create_proc_file(int cpid, char * file_path, char * user_info) {
	strcat(file_path, "/pseudo_proc");
    makedir(file_path);
	char * temp_file = malloc(sizeof(char) * 200);
	sprintf(temp_file, "%s/%d", file_path, cpid);
	sprintf(file_path, "%s", temp_file);
	makedir(file_path);
	sprintf(temp_file, "%s/attr", file_path);
	sprintf(file_path, "%s", temp_file);
	makedir(file_path);
	sprintf(temp_file, "%s/current", file_path);
	sprintf(file_path, "%s", temp_file);
	FILE * procFP = fopen(file_path, "w");
	fprintf(procFP, "%s", user_info);
	fclose(procFP);
}

/**
 * makedir - make a directory if it does not already exist
 * path - the path to the directory to make
*/
void makedir(char * path) {
	struct stat st = {0};
	if (stat(path, &st) == -1) {
		if (mkdir(path, 0777) == -1) {
			fprintf(stderr, "Error creating directory: %s\n", path);
		}
	}
}

/**
 * get_user_from_db - get the hierarchical info of the user from the targeted
 * users database
 * username - the username of the user that is signed in
 * targeted_users_db_file - the filepointer of the targeted users DB
 */
char * get_user_from_db(char * username, FILE * targeted_users_db_file) {
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	while((read = getline(&line, &len, targeted_users_db_file)) != -1) {
		char * token = strtok(strdup(line), ":");
		if(strcmp(token, username) == 0) {
			free(token);
			return strdup(line);
		}
	}
	return NULL;
}
/**
 * pam_sm_authenticate - executes on PAM auth keyword such as in system-auth. Creates
 * 	a pseudo-proc file which contains signed in user's PAMEx classification information
*/
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	char *username = NULL;
	int cpid = getpid();
	char * file_path = malloc(200 * sizeof(char));
	
	// Get the username
	if (pam_get_user(pamh, (const char **)&username, NULL) != PAM_SUCCESS || username == NULL) {
		fprintf(stderr, "Cannot find username %s\n", username);
		return PAM_IGNORE;
	}
	printf("check args\n");
	// check args are valid
	if(argc != 2) {
		printf("usage: <path_to_users_db> <dest_dir>");
		return PAM_IGNORE;
	}
	// find targeted user db file and create file pointer
	char * targeted_users_db_path = strdup(argv[0]);
	FILE * targeted_users_db_file = fopen(targeted_users_db_path, "r");
	printf("get targeted users db\n");
	if(!targeted_users_db_file) {
		fprintf(stderr, "No targeted users DB file at %s\n", targeted_users_db_path);
		return PAM_IGNORE;
	}
	// get the user information of the current signed in user from the db
	char * user_info = strdup(get_user_from_db(username, targeted_users_db_file));
	fclose(targeted_users_db_file);
	printf("get user info\n");
	if(user_info == NULL) {
		// If the user is not found in the user DB
		// give them level unrestricted
		char * user_info_temp = malloc(500);
		sprintf(user_info_temp, "%s:unrestricted:0", username);
		user_info = strdup(user_info_temp);
	}
	
	file_path = strdup(argv[1]);
	printf("create proc\n");
	create_proc_file(cpid, file_path, user_info);
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