/**
 * This is the file labeler tool for Pamx
 * The job of this tool is to allow a privileged user to change a file's
 *  Pamx levels and labels via their extended attributes easily
 * 
 * Author: Daniel Weninger
 * Last Modified: 4/29/2023
*/

#include "file-labeler.h"

int main (int argc, char ** argv) {
    int retval = 0;
    if(argc != 5) {
        fprintf(stderr, "usage: %s <path_to_level_db> <path_to_file> (-al | -cl | -rl | -ac | -rc) <level_or_label_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
     char * path_to_level_db = argv[1];
     char * path_to_file = argv[2];
     char * flag = argv[3];
     char * name = argv[4];
    // Determine operation based on flag
    if(strcmp(flag, "-al") == 0) {
        // add level
        add_level(path_to_level_db, path_to_file, name);
    } else if(strcmp(flag, "-cl") == 0) {
        // change level
        add_level(path_to_level_db, path_to_file, name);
    } else if(strcmp(flag, "-rl") == 0) {
        // remove level
        remove_level(path_to_file);
    } else if(strcmp(flag, "-ac") == 0) {
        // add label
        add_label(path_to_file, name);
    } else if(strcmp(flag, "-rc") == 0) {
        // remove label
        remove_label(path_to_file, name);
    } else {
        fprintf(stderr, "Unable to interpret flag.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * add_level - add a new level to a file's extended attributes. If
 *  file has a level, replace it
 * level_db_path - path to the level database file to check if level
 *  being added is in the database
 * file_path - the path to the file that the level is being added to
 * level_name - the name of the level to add to the file
 * returns  - 1 if level added or 0 if not added
*/
int add_level(char * level_db_path, char * file_path, char * level_name) {
    FILE * level_db_fp = fopen(level_db_path, "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int found_level = 0;
    char * db_line = malloc(sizeof(char) * 250);
    // Check if level database exists
    if (level_db_fp == NULL) {
        fprintf(stderr, "Unable to open level db from %s\n", level_db_path);
        exit(EXIT_FAILURE);
    }
    // Find the level proposed in the level database
    while ((read = getline(&line, &len, level_db_fp)) != -1) {
        char * token = strtok(strdup(line), ":");
        if(strcmp(token, level_name) == 0) {
            found_level = 1;
            strcpy(db_line, line);
            db_line[strcspn(db_line, "\n")] = '\0';
        }
    }
    // Level not found in database
    if(!found_level){
        fprintf(stderr, "Could not find level %s in the level database\n", level_name);
        exit(EXIT_FAILURE);
    }

    fclose(level_db_fp);
    // Set the level of the file to be the new level
    if(setxattr(file_path, "security.pamex.level", db_line, strlen(db_line), 0) == -1) {    
		fprintf(stderr, "Error setting level attribute %s for file %s - %s\n", db_line, file_path, strerror(errno));
		exit(EXIT_FAILURE);
    }
    return 1;
}

/**
 * remove_level - remove the level from file's extended attributes. If
 *  the file does not have a level, do nothing
 * file_path - the path to the file that the level is being removed from
 * returns  - 1 if level removed or 0 if not removed
*/
int remove_level(char * file_path) {
    // Remove the extended attribute of level from the file
    if(removexattr(file_path, "security.pamex.level") == -1) {  
		fprintf(stderr, "Error removing level attribute for file %s - %s\n", file_path, strerror(errno));
		exit(EXIT_FAILURE);
    }
    return 1;
}

/**
 * add_label - add a new label to a file's extended attributes. If
 *  file has that label, do nothing
 * file_path - the path to the file that the label is being added to
 * label_name - the name of the label to add to the file
 * returns  - 1 if label added or 0 if not added
*/
int add_label(char * file_path, char * label_name) {
    char ** file_labels = get_file_labels(file_path);
    // Check if the file already has the label and exit if so
    if(contains_label(file_labels, label_name)) {
        exit(EXIT_SUCCESS);
    }
    char * new_labels = malloc(500);
    char * xattr = malloc(500);
    int xattr_size = getxattr(file_path, "security.pamex.labels", xattr, 500);
	if(xattr_size == -1) {
		if(errno == ENODATA) {
			sprintf(new_labels, "%s", label_name);
		} else {
			fprintf(stderr, "Error getting label attributes for file %s - %s\n", file_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else {
        // Append the new label to existing labels
        sprintf(new_labels, "%s:%s", xattr, label_name);
    }
    // Set the label attribute to be new label + existing labels
    if(setxattr(file_path, "security.pamex.label", new_labels, strlen(new_labels), 0) == -1) {    
		fprintf(stderr, "Error setting level attribute %s for file %s - %s\n", new_labels, file_path, strerror(errno));
		exit(EXIT_FAILURE);
    }
    return 1;
}

/**
 * remove_label - remove a label from file's extended attributes. If
 *  the file does not have that label, do nothing
 * file_path - the path to the file that the label is being removed from
 * label_name - the name of the label to remove from the file's extended attribute
 * returns  - 1 if label removed or 0 if not removed
*/
int remove_label(char * file_path, char * label_name) {
    // Get all of the labels of the file except for the label to be removed
    char ** file_labels = get_file_labels_except(file_path, label_name);
    char * new_labels = malloc(500);
    int i = 0;
    // Concatenate the new list of labels with the label removed
    if(file_labels && file_labels[i] && strcmp(file_labels[i], "") != 0) {
        strcat(new_labels, strdup(file_labels[i]));
        i++;
    }

    while(file_labels && file_labels[i] && strcmp(file_labels[i], "") != 0) {
        file_labels[i][strcspn(file_labels[i], "\n")] = 0;
        strcat(new_labels, ":");
        strcat(new_labels, strdup(file_labels[i]));
        i++;
	}
    // Set the labels attribute to the new list of labels
    if(setxattr(file_path, "security.pamex.labels", new_labels, strlen(new_labels), 0) == -1) {    
		fprintf(stderr, "Error setting level attribute %s for file %s - %s\n", new_labels, file_path, strerror(errno));
		exit(EXIT_FAILURE);
    }
    return 1;
}

/**
 * get_file_labels - get the value of the extended attribute security.pamex.labels from
 *  a file as a list of strings
 * targeted_file_path - the path to the file to get the list of labels from
 * returns  - a list of the labels from the file specified
*/
char ** get_file_labels(char * targeted_file_path) {
	char * xattr = malloc(500);
	char ** label_list = (char**)malloc(sizeof(char*));
	int index = 0;
    // Get the labels attribute value
	int xattr_size = getxattr(targeted_file_path, "security.pamex.labels", xattr, 500);
	if(xattr_size == -1) {
		if(errno == ENODATA) {
			return NULL;
		} else {
			fprintf(stderr, "Error getting label attributes for file %s - %s\n", targeted_file_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	// Make a list of strings of the labels with a colon delimiter
	char * token = strtok(xattr, ":");
	while(token) {
		label_list = (char **)realloc(label_list, (index +1) * sizeof(char*));
		label_list[index] = strdup(token);
		token = strtok(NULL, ":");
		index++;
	}
	return label_list;
}

/**
 * get_file_labels - get the value of the extended attribute security.pamex.labels from
 *  a file as a list of strings but exclude a specified label from the list
 * targeted_file_path - the path to the file to get the list of labels from
 * label_name - the label to exclude from the list
 * returns  - a list of the labels excluding the one specified from the file specified
*/
char ** get_file_labels_except(char * targeted_file_path, char * label_name) {
	char * xattr = malloc(500);
	char ** label_list = (char**)malloc(sizeof(char*));
	int index = 0;
    // Get all of the labels of the file
	int xattr_size = getxattr(targeted_file_path, "security.pamex.labels", xattr, 500);
	if(xattr_size == -1) {
		if(errno == ENODATA) {
			return NULL;
		} else {
			fprintf(stderr, "Error getting label attributes for file %s - %s\n", targeted_file_path, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	// Make a list of strings of the labels but skip over the label with the name specified
	char * token = strtok(xattr, ":");
	while(token) {
        if(strcmp(token, label_name) != 0) {
            label_list = (char **)realloc(label_list, (index +1) * sizeof(char*));
            label_list[index] = strdup(token);
            token = strtok(NULL, ":");
            index++;
        }
	}
	return label_list;
}

/**
 * contains_label - check if a label is contained in a list of labels
 * label_list - a string list of labels
 * ref_label - the string to search for
 * returns  - 1 if ref_label found in label_list or 0 if not found
*/
int contains_label(char ** label_list, char * ref_label) {
	int i = 0;
    // Search through the label list and return 1 if ref_label is found
	while(label_list && label_list[i] && strcmp(label_list[i], "") != 0) {
        label_list[i][strcspn(label_list[i], "\n")] = 0;
		if(strcmp(label_list[i], ref_label) == 0) {
			return 1;
		}	
        i++;		
	}
    // ref_label is not found
	return 0;
}