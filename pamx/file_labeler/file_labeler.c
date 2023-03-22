#include <sys/xattr.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include "file_labeler.h"

int main (int argc, char ** argv) {
    int retval = 0;
    if(argc != 5) {
        fprintf(stderr, "usage: %s <path_to_level_db> <path_to_file> (-al | -cl | -rl | -ac | -rc) <level_or_label_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
     char * PATH_TO_LEVEL_DB = argv[1];
     char * PATH_TO_FILE = argv[2];
     char * FLAG = argv[3];
     char * NAME = argv[4];

    if(strcmp(FLAG, "-al") == 0) {
        // add level
        addlevel(PATH_TO_LEVEL_DB, PATH_TO_FILE, NAME);
    } else if(strcmp(FLAG, "-cl") == 0) {
        // change level
        addlevel(PATH_TO_LEVEL_DB, PATH_TO_FILE, NAME);
    } else if(strcmp(FLAG, "-rl") == 0) {
        // remove level
        removelevel(PATH_TO_FILE);
    } else if(strcmp(FLAG, "-ac") == 0) {
        // add label
        addlabel(PATH_TO_FILE, NAME);
    } else if(strcmp(FLAG, "-rc") == 0) {
        // remove label
        removelabel(PATH_TO_FILE, NAME);
    } else {
        fprintf(stderr, "Unable to interpret flag.\n");
    }
}

int addlevel(char * level_db_path, char * file_path, char * level_name) {
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
            db_line = strdup(line);
        }
    }

    if(!found_level){
        fprintf(stderr, "Could not find level %s in the level database\n", level_name);
        exit(EXIT_FAILURE);
    }

    fclose(level_db_fp);
    
    if(setxattr(file_path, "security.fsc.level", db_line, strlen(db_line), 0) == -1) {    
		setxattrErrorPrints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", db_line, file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

int removelevel(char * file_path) {
    if(removexattr(file_path, "security.fsc.level") == -1) {  
		fprintf(stderr, "Error removing level attribute for file %s - Errno: %d\n", file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

int addlabel(char * file_path, char * label_name) {
    char ** file_labels = getfilelabels(file_path);
    if(!containslabel(file_labels, label_name)) {
        exit(EXIT_SUCCESS);
    }
    char * new_labels = malloc(500);
    char * xattr = malloc(500);
    int xattrSize = getxattr(file_path, "security.fsc.labels", xattr, 500);
	if(xattrSize == -1) {
		if(errno == ENODATA) {
			return 0;
		} else {
			getxattrErrorPrints();
			fprintf(stderr, "Error getting label attributes for file %s - Errno: %d\n", file_path, errno);
			exit(EXIT_FAILURE);
		}
	}

    sprintf(new_labels, "%s:%s", xattr, label_name);

    if(setxattr(file_path, "security.fsc.label", new_labels, strlen(new_labels), 0) == -1) {    
		setxattrErrorPrints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", new_labels, file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

int removelabel(char * file_path, char * label_name) {
    char ** file_labels = getfilelabelsexcept(file_path, label_name);
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
		setxattrErrorPrints();
		fprintf(stderr, "Error setting level attribute %s for file %s - Errno: %d\n", new_labels, file_path, errno);
		exit(EXIT_FAILURE);
    }
    return 1;
}

char ** getfilelabels(char * targetedFilePath) {
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

char ** getfilelabelsexcept(char * targetedFilePath, char * labelName) {
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
        if(strcmp(token, labelName) != 0) {
            labelList = (char **)realloc(labelList, (index +1) * sizeof(char*));
            labelList[index] = strdup(token);
            token = strtok(NULL, ":");
            index++;
        }
	}
	return labelList;
}

int containslabel(char ** label_list, char * ref_label) {
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

void setxattrErrorPrints() {
	printf("EDQUOT: %d\nEExist XATTR_CREATE: %d\nENODATA XATTR_REPLACE: %d\nENOSPC:%d\nENOTSUP: %d\nEPERM: %d\nERANGE: %d\n", EDQUOT, EEXIST, ENODATA, ENOSPC, ENOTSUP, EPERM, ERANGE);
}
	
void getxattrErrorPrints() {
	printf("E2BIG: %d\nENODATA: %d\nENOTSUP: %d\nERANGE:%d\n", E2BIG, ENODATA, ENOTSUP, ERANGE);
}