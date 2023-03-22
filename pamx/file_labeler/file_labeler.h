int addlevel(char * level_db_path, char * file_path, char * level_name);
int removelevel(char * file_path);
int addlabel(char * file_path, char * label_name);
int removelabel(char * file_path, char * label_name);
char ** getfilelabels(char * targetedFilePath);
char ** getfilelabelsexcept(char * targetedFilePath, char * labelName);
int containslabel(char ** label_list, char * ref_label);
void setxattrErrorPrints();
void getxattrErrorPrints();