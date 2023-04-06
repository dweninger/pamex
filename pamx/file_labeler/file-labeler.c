/**
 * This is the file labeler tool for Pamx
 * The job of this tool is to allow a privileged user to change a file's
 *  Pamx levels and labels via their extended attributes easily
 * 
 * Author: Daniel Weninger
 * Last Modified: 3/22/2023
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

int add_level(char * level_db_path, char * file_path, char * level_name) {
    FILE * level_db_fp = fopen(level_db_path, "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int found_level = 0;
    char * db_line = malloc(sizeof(char) * 250);

    if (level_db_fp == NULL) {
        fprintf(stderr, "Unable to open level db from %s\n", level_db_path);
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, level_db_fp)) != -1) {
        char * token = strtok(strdup(line), ":");
        if(strcmp(token, level_name) == 0) {
            found_level = 1;
            strcpy(db_line, line);
            db_line[strcspn(db_line, "\n")] = '\0';
        }
    }

    if(!found_level){
        fprintf(stderr, "Could not find level %s in the level database\n", level_name);
        exit(EXIT_FAILURE);
    }

    fclose(level_db_fp);
    
    if(setxattr(file_path, "security.fsc.level", db_line, strlen(db_line), 0) == -1) {    
		setxattr_error_prints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", db_line, file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

int remove_level(char * file_path) {
    if(removexattr(file_path, "security.fsc.level") == -1) {  
		fprintf(stderr, "Error removing level attribute for file %s - Errno: %d\n", file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

int add_label(char * file_path, char * label_name) {
    char ** file_labels = get_file_labels(file_path);
    if(contains_label(file_labels, label_name)) {
        exit(EXIT_SUCCESS);
    }
    char * new_labels = malloc(500);
    char * xattr = malloc(500);
    int xattr_size = getxattr(file_path, "security.fsc.labels", xattr, 500);
	if(xattr_size == -1) {
		if(errno == ENODATA) {
			sprintf(new_labels, "%s", label_name);
		} else {
			getxattr_error_prints();
			fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", file_path, errno);
			exit(EXIT_FAILURE);
		}
	} else {
        sprintf(new_labels, "%s:%s", xattr, label_name);
    }

    if(setxattr(file_path, "security.fsc.label", new_labels, strlen(new_labels), 0) == -1) {    
		setxattr_error_prints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", new_labels, file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

int remove_label(char * file_path, char * label_name) {
    char ** file_labels = get_file_labels_except(file_path, label_name);
    char * new_labels = malloc(500);
    int i = 0;

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
     if(setxattr(file_path, "security.fsc.label", new_labels, strlen(new_labels), 0) == -1) {    
		setxattr_error_prints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", new_labels, file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
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

char ** get_file_labels_except(char * targeted_file_path, char * label_name) {
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
        if(strcmp(token, label_name) != 0) {
            label_list = (char **)realloc(label_list, (index +1) * sizeof(char*));
            label_list[index] = strdup(token);
            token = strtok(NULL, ":");
            index++;
        }
	}
	return label_list;
}

int contains_label(char ** label_list, char * ref_label) {
	int i = 0;
    
	while(label_list && label_list[i] && strcmp(label_list[i], "") != 0) {
        label_list[i][strcspn(label_list[i], "\n")] = 0;
		if(strcmp(label_list[i], ref_label) == 0) {
			return 1;
		}	
        i++;		
	}
	return 0;
}

void setxattr_error_prints() {
	printf("EDQUOT: %d\nEExist XATTR_CREATE: %d\nENODATA XATTR_REPLACE: %d\nENOSPC:%d\nENOTSUP: %d\nEPERM: %d\nERANGE: %d\n", EDQUOT, EEXIST, ENODATA, ENOSPC, ENOTSUP, EPERM, ERANGE);
}
	
void getxattr_error_prints() {
	printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}