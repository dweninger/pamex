#include <sys/xattr.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

int get_user_level(char * proc_file_path, char * level_db_path);
char ** get_user_labels(char * proc_file_path);
int get_file_level(char * targeted_file_path, char * level_db_path);
char ** get_file_labels(char * targeted_file_path);
int contains_labels(char ** ref_labels, char ** user_labels);
