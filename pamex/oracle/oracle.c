/**
 * This is the Oracle tool for PAMEx
 * The job of this tool is to simulate how a custom kernel would cross reference
 *  the logged in user's credentials via the proc directory and the exteneded
 *  attributes to ensure that a user is allowed to access the file. The Oracle
 *  tool simulates this by checking a custom-made proc-like file and the file's
 *  extended attributes.
 * 
 * Author: Daniel Weninger
 * Last Modified: 4/29/2023
*/

#include "oracle.h"

int main (int argc, char ** argv) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char user_command[100];

    // Check args
	if(argc != 3) {
		fprintf(stderr, "usage: %s <dir_containing_pseudo_proc> <path_to_level_db>\n", argv[0]);
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

    printf("Welcome to the PAMEx Oracle!\n");

    // Prompt user for PID
    // This is the name of the dir that the attr/current file is in
    printf("Enter PID\n");
    printf(" > ");
    scanf("%s", user_command); 
	
	// Set proc path based on user input
	char * proc_file_path = malloc(200 * sizeof(char));
    sprintf(proc_file_path, "%s/pseudo_proc/%s/attr/current", argv[1], user_command);

    // Make sure proc file exists
    FILE * proc_file = fopen(proc_file_path, "r");
    // If the proc file was not found, prompt for it again
    while(!proc_file) {
        printf("Proc file not found for pid %s. Try again.\n", user_command);
        printf(" > ");
        scanf("%s", user_command);
        sprintf(proc_file_path, "%s/pseudo_proc/%s/attr/current", argv[1], user_command);
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
            printf("PAMEx Oracle Commands:\n"
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
	        int xattr_size = 0;

            FILE * targeted_file = fopen(targeted_file_path, "r");
            if(!targeted_file) {
                fprintf(stderr, "File not found at %s\n", targeted_file_path);
                goto LOOP;
            }
            fclose(targeted_file);

            char * level_name = "unclassified";
            int level_placement = 0;
	        char level_xattr[501]; // allocate an extra byte for the null-terminator
            xattr_size = getxattr(targeted_file_path, "security.pamex.level", level_xattr, 500);
            level_xattr[xattr_size] = '\0'; // manually null-terminate the buffer
            if(xattr_size != -1) {
                char * level_token = strtok(strdup(level_xattr), ":");
                level_name = strdup(level_token);
		        level_token = strtok(NULL, ":");
                level_placement = get_file_level(targeted_file_path, level_db_path);
            	level_token = "\0";
	        }
            
            printf("{\"file\" : \"%s\", \"level\" : \"%s\", \"placement\" : \"%d\", \"labels\" : [", targeted_file_path, level_name, level_placement);
            
            char label_xattr[501]; // allocate an extra byte for the null-terminator
            xattr_size = getxattr(targeted_file_path, "security.pamex.labels", label_xattr, 500);
            label_xattr[xattr_size] = '\0'; // manually null-terminate the buffer

            if(xattr_size != -1) {
               	char * label_token = strtok(label_xattr, ":");
                while(label_token) {
                    printf("{\"name\" : \"%s\"}, ", label_token);
		            label_token = "\0";
                    label_token = strtok(NULL, ":");
                } 
            }
            printf("]}\n");
        } else {
            printf("Not a valid command. Type help for a list of commands.\n");
        }
	//

    LOOP:
        printf(" > ");
        scanf("%s", user_command); 
    }

}

/**
 * getUserLevel - get the hierarchical level of the user from the pseudo proc file
 * targetedUsersDBFile - the filepointer of the targeted users DB 
 * returns  - 1 if user level found and 0 if not
 */
int get_user_level(char * proc_file_path, char * level_db_path) {
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * level_name = malloc(200);
    int placement = -1;
    FILE * proc_file = fopen(proc_file_path, "r");
    FILE * level_db_file = fopen(level_db_path, "r");
    // Read through the pseudo proc info to find the user level
	read = getline(&line, &len, proc_file);
    char * token = strtok(strdup(line), ":");
    token = strtok(NULL, ":");
    strcpy(level_name, token);
    // Get the level's placement from the level database
    while ((read = getline(&line, &len, level_db_file)) != -1) {
        char * level_token = strtok(strdup(line), ":");
        if(strcmp(level_name, level_token) == 0) {
            level_token = strtok(NULL, ":");
            placement = atoi(level_token);
        }
    }
    // Level not found in the level database
    if(placement == -1) {
        fprintf(stderr, "Could not find user's level %s in the level DB\n", level_name);
        exit(EXIT_FAILURE);
    }

    fclose(proc_file);
    fclose(level_db_file);
	return placement;
}

/**
 * get_user_labels - Given a path to a file in the proc directory, returns an array of the user's labels.
 * 
 * proc_file_path - the path to the file in the proc directory
 * returns  - an array of the user's labels
 */
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


/**
 * get_file_level - Given a path to a file and a path to the level database, returns the file's
 * clearance level as an integer.
 * 
 * file_path - the path to the file
 * level_db_path - the path to the level database
 * returns  - the file's clearance level as an integer
 */
int get_file_level(char * targeted_file_path, char * level_db_path) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char * level_name = malloc(200);
    int placement = -1;
	char * xattr = malloc(500);

    int xattr_size = getxattr(targeted_file_path, "security.pamex.level", xattr, 500);
    if(xattr_size == -1) {
        return 0;
    }

	char * token = strtok(xattr, ":");
    strcpy(level_name, token);
    FILE * level_db_file = fopen(level_db_path, "r");
    // Find the level in the level database
    while ((read = getline(&line, &len, level_db_file)) != -1) {
        char * level_token = strtok(strdup(line), ":");
        // When the level is found, extract its placement
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

/**
 * get_file_labels - Given a path to a file, returns an array of the file's labels.
 * 
 * file_path - the path to the file
 * returns  - an array of the file's labels
 */
char ** get_file_labels(char * targeted_file_path) {
    char xattr[501]; // allocate an extra byte for the null-terminator
    int xattr_size = getxattr(targeted_file_path, "security.pamex.labels", xattr, 500);
    xattr[xattr_size] = '\0'; // manually null-terminate the buffer
	char ** label_list = (char**)malloc(sizeof(char*));
	int index = 0;
	printf("xattr: %s\n", xattr);
    strcat(xattr, ":\0");
    printf("xattr: %s\n", xattr);
	if(xattr_size == -1) {
		if(errno == ENODATA) {
			return NULL;
		} else {
			fprintf(stderr, "Error getting label attributes for file %s - %s\n", targeted_file_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	
	char * token = strtok(xattr, ":");
    printf("token: %s\n", token);
	while(token) {
        
		label_list = (char **)realloc(label_list, (index +1) * sizeof(char*));
		label_list[index] = strdup(token);
		token = strtok(NULL, ":");
		index++;
	}
    label_list[index] = "\0";
	return label_list;
}
 /**
  * contains_labels - Given two lists of labels, check if list b is contained in list a
  * ref_labels - the labels to check if the user has
  * user_labels - the labels assigned to the user
  * returns  - 1 if user has all of the labels in ref_labels or 0 otherwise
 */
int contains_labels(char ** ref_labels, char ** user_labels) {
	int ref_i = 0;
	int user_i = 0;
	int found_match = 0;
    // Check that for each ref label, there is a user label that has the same name
	while(ref_labels && ref_labels[ref_i] && strcmp(ref_labels[ref_i], "") != 0) {
        found_match = 0; 
		user_i = 0;
        printf("ref_label: %s\n",ref_labels[ref_i]);
		while(user_labels && user_labels[user_i] && strcmp(user_labels[user_i], "") != 0) {
            printf("user_label: %s\n", user_labels[user_i]);
            user_labels[user_i][strcspn(user_labels[user_i], "\n")] = 0;
			if(strcmp(ref_labels[ref_i], user_labels[user_i]) == 0) {
				found_match = 1;
				break;
			}			
			user_i++;	
		}
        // User does not have a label that it needs
		if(!found_match) {
			return 0;
		}
		ref_i++;
	}
    // User has all of the needed labels
	return 1;
}
