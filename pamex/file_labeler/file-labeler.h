#include <sys/xattr.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

int add_level(char * level_db_path, char * file_path, char * level_name);
int remove_level(char * file_path);
int add_label(char * file_path, char * label_name);
int remove_label(char * file_path, char * label_name);
char ** get_file_labels(char * targeted_file_path);
char ** get_file_labels_except(char * targeted_file_path, char * label_name);
int contains_label(char ** label_list, char * ref_label);
