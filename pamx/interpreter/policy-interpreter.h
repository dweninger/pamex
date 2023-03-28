
void assign_level_to_file(char * assignee, char * assignment_data, char * path_to_files);
void assign_label_to_file(char * assignee, char * assignment_data, char * path_to_files);
int label_exists_in_xattrs(char * check_label, char * file_json);
int file_already_accessed(char * file);
void write_user_level_to_db(char * assignee, char * assignment_data);
void write_user_label_to_db(char * assignee, char * assignment_data);
void setxattr_error_prints();
void getxattr_error_prints();
