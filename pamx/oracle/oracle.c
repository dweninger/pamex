/**
 * This is the Oracle tool for Pamx
 * The job of this tool is to simulate how a custom kernel would cross reference
 *  the logged in user's credentials via the proc directory and the exteneded
 *  attributes to ensure that a user is allowed to access the file. The Oracle
 *  tool simulates this by checking a custom-made proc-like file and the file's
 *  extended attributes.
 * 
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

#include "oracle.h"

int main (int argc, char ** argv) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char user_command[100];

    // Check args
	if(argc != 3) {
		fprintf(stderr, "usage: %s <dir_containing_sudo_proc> <path_to_level_db>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    // Make sure level database exists based on user args
    char * level_db_path = strdup(argv[2]);
    FILE * level_db = fopen(level_db_path, "r");
    if(!level_db) {
        fprintf(stderr, "Could not find level DB\n");
        exit(EXIT_FAILURE);
    }
    fclose(level_db);

    printf("Welcome to the Pamx Oracle!\n");

    // Prompt user for PID
    // This is the name of the dir that the attr/current file is in
    printf("Enter PID\n");
    printf(" > ");
    scanf("%s", user_command); 
	
	// Set proc path based on user input
	char * proc_file_path = malloc(200 * sizeof(char));
    sprintf(proc_file_path, "%s/sudo_proc/%s/attr/current", argv[1], user_command);

    // Make sure proc file exists
    FILE * proc_file = fopen(proc_file_path, "r");
    // If the proc file was not found, prompt for it again
    while(!proc_file) {
        printf("Proc file not found for pid %s. Try again.\n", user_command);
        printf(" > ");
        scanf("%s", user_command);
        sprintf(proc_file_path, "%s/sudo_proc/%s/attr/current", argv[1], user_command);
        proc_file = fopen(proc_file_path, "r");
    }
    // Check that the proc file contains information
    if(read = getline(&line, &len, proc_file) == -1) {
        fprintf(stderr, "The proc file is empty\n");
        exit(EXIT_FAILURE);
    }
    fclose(proc_file);
    
    printf("Pid accepted.\n");

    // Keep prompting user for commands until they quit
    printf("Type help for a list of commands.\n");
    printf(" > ");
    scanf("%s", user_command); 
    while(strcasecmp(user_command, "quit") != 0) {
        
        if(strcasecmp(user_command, "help") == 0) {
            // help
            printf("Pamx Oracle Commands:\n"
            "help - print list of commands\n"
            "user - check the name of the signed in user\n"
            "userinfo - get the name and authentication information of signed in user\n"
            "checkfileaccess - check if the currently signed in user can access a file\n"
            "cfa - alias for the checkfileaccess command\n"
            "fileinfo - get the authentication information of a file\n"
            "quit - quit the Oracle\n");
        
        } else if(strcasecmp(user_command, "user") == 0) {
            // check username
            proc_file = fopen(proc_file_path, "r");
            read = getline(&line, &len, proc_file);
            char * token = strtok(strdup(line), ":");
            printf("%s\n", token);
            free(token);
            fclose(proc_file);
        
        } else if (strcasecmp(user_command, "userinfo") == 0) {
            // check user info
            proc_file = fopen(proc_file_path, "r");
            read = getline(&line, &len, proc_file);

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
            fclose(proc_file);
        
        } else if (strcasecmp(user_command, "checkfileaccess") == 0 || strcasecmp(user_command, "cfa") == 0) {
            // check if a user can access a file
            int user_level = get_user_level(proc_file_path, level_db_path);
            char ** user_labels = get_user_labels(proc_file_path);
            printf("File path: ");
            char targeted_file_path[100];
            scanf("%s", targeted_file_path);

            FILE * targeted_file = fopen(targeted_file_path, "r");
            if(!targeted_file) {
                fprintf(stderr, "File not found at %s\n", targeted_file_path);
                goto LOOP;
            }
            fclose(targeted_file);

            int file_level = get_file_level(targeted_file_path, level_db_path);
	        char ** file_labels = get_file_labels(targeted_file_path);

            if(user_level >= file_level) {
                if(contains_labels(file_labels, user_labels)) {
                    printf("\nAccess granted!\n");
                } else {
                    printf("\nAccess denied. User does not have appropriate labels.\n");
                }
            } else {
                printf("\nAccess denied. User level too low.\n");
            }
        
        } else if (strcasecmp(user_command, "fileinfo") == 0) {
            // check file info
            printf("File path: ");
            char targeted_file_path[100];
            scanf("%s", targeted_file_path);

            FILE * targeted_file = fopen(targeted_file_path, "r");
            if(!targeted_file) {
                fprintf(stderr, "File not found at %s\n", targeted_file_path);
                goto LOOP;
            }
            fclose(targeted_file);

            char * level_name = "unclassified";
            int level_placement = 0;
            char * level_xattr = malloc(500);
            int xattr_size = getxattr(targeted_file_path, "security.fsc.level", level_xattr, 500);
            if(xattr_size != -1) {
                char * token = strtok(level_xattr, ":");
                level_name = strdup(token);
	            token = strtok(NULL, ":");
                level_placement = get_file_level(targeted_file_path, level_db_path);
            }

            printf("{\"file\" : \"%s\", \"level\" : \"%s\", \"placement\" : \"%d\", \"labels\" : [", targeted_file_path, level_name, level_placement);
            char * label_xattr = malloc(500);
            xattr_size = getxattr(targeted_file_path, "security.fsc.labels", label_xattr, 500);
            if(xattr_size != -1) {
               char * token = strtok(label_xattr, ":");
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
        scanf("%s", user_command); 
    }

}

/**
 * getUserLevel - get the hierarchical level of the user from the sudo proc file
 * targetedUsersDBFile - the filepointer of the targeted users DB 
 */
int get_user_level(char * proc_file_path, char * level_db_path) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * level_name = malloc(200);
    int placement = -1;
    FILE * proc_file = fopen(proc_file_path, "r");
    FILE * level_db_file = fopen(level_db_path, "r");

	read = getline(&line, &len, proc_file);
    char * token = strtok(strdup(line), ":");
    token = strtok(NULL, ":");
    strcpy(level_name, token);

    while ((read = getline(&line, &len, level_db_file)) != -1) {
        char * level_token = strtok(strdup(line), ":");
        if(strcmp(level_name, level_token) == 0) {
            level_token = strtok(NULL, ":");
            placement = atoi(level_token);
        }
    }

    if(placement == -1) {
        fprintf(stderr, "Could not find user's level %s in the level DB\n", level_name);
        exit(EXIT_FAILURE);
    }

    fclose(proc_file);
    fclose(level_db_file);
	return placement;
}

char ** get_user_labels(char * proc_file_path) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
	char ** label_list = (char**)malloc(sizeof(char*));
	int index = 0;
    FILE * proc_file = fopen(proc_file_path, "r");

    read = getline(&line, &len, proc_file);
    char * token = strtok(strdup(line), ":");
    token = strtok(NULL, ":");
    token = strtok(NULL, ":");
	token = strtok(NULL, ":");

    while(token) {
        label_list = (char**)realloc(label_list, (index + 1) * sizeof(char *));
        label_list[index] = strdup(token);
        index++;
        token = strtok(NULL, ":");
    }

    fclose(proc_file);
    return label_list;
}

int get_file_level(char * targeted_file_path, char * level_db_path) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * level_name = malloc(200);
    int placement = -1;
	char * xattr = malloc(500);

    int xattr_size = getxattr(targeted_file_path, "security.fsc.level", xattr, 500);
    if(xattr_size == -1) {
        return 0;
    }

	char * token = strtok(xattr, ":");
    strcpy(level_name, token);
    FILE * level_db_file = fopen(level_db_path, "r");
    while ((read = getline(&line, &len, level_db_file)) != -1) {
        char * level_token = strtok(strdup(line), ":");
        if(strcmp(level_name, level_token) == 0) {
            level_token = strtok(NULL, ":");
            placement = atoi(level_token);
        }
    }
    
    if(placement == -1) {
        fprintf(stderr, "Could not find user's level %s in the level DB\n", level_name);
        exit(EXIT_FAILURE);
    }

    fclose(level_db_file);
	return placement;
}

char ** get_file_labels(char * targeted_file_path) {
	char * xattr = malloc(500);
	char ** label_list = (char**)malloc(sizeof(char*));
	int index = 0;

	int xattr_size = getxattr(targeted_file_path, "security.fsc.labels", xattr, 500);
	if(xattr_size == -1) {
		if(errno == ENODATA) {
			return NULL;
		} else {
			getxattr_error_prints();
			fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", targeted_file_path, errno);
			exit(EXIT_FAILURE);
		}
	}
	
	char * token = strtok(xattr, ":");
	while(token) {
		label_list = (char **)realloc(label_list, (index +1) * sizeof(char*));
		label_list[index] = strdup(token);
		token = strtok(NULL, ":");
		index++;
	}
	return label_list;
}

int contains_labels(char ** ref_labels, char ** user_labels) {
	int ref_i = 0;
	int user_i = 0;
	int found_match = 0;
	while(ref_labels && ref_labels[ref_i] && strcmp(ref_labels[ref_i], "") != 0) { 
		user_i = 0;
		while(user_labels && user_labels[user_i] && strcmp(user_labels[user_i], "") != 0) {
            user_labels[user_i][strcspn(user_labels[user_i], "\n")] = 0;
			if(strcmp(ref_labels[ref_i], user_labels[user_i]) == 0) {
				found_match = 1;
				break;
			}			
			user_i++;	
		}
		if(!found_match) {
			return 0;
		}
		found_match = 0;
		ref_i++;
	}
	return 1;
}

void getxattr_error_prints() {
        printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}
