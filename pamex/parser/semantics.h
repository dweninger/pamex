void yyerror(char * s);

void do_user_assign_level(char * file_path, char * levelName, char * user);
void do_user_assign_labels(char * file_path, char ** labelList, char * user);
void do_file_assign_level(char * file_path, char * levelName, char * file);
void do_file_assign_labels(char * file_path, char ** labelList, char * file);
char ** do_label_list(char * labels);
char * do_concat_labels(char * label);
void do_define_level(char * levelName, int placement, char * level_db_path);
void do_define_label(char * labelName);
int do_comp(int op, char * id);
int do_set(int res, char * level_db_path);
void shift_level_placements(int pos);
void print_level_placements(char * level_db_path);
void Finish();
