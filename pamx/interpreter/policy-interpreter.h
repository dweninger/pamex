
void assignLevelToFile(char * assignee, char * assignmentData, char * pathToFiles);
void assignLabelToFile(char * assignee, char * assignmentData, char * pathToFiles);
int labelExistsInXAttrs(char * checkLabel, char * fileJson);
int fileAlreadyAccessed(char * file);
void writeUserLevelToDB(char * assignee, char * assignmentData);
void writeUserLabelToDB(char * assignee, char * assignmentData);
void setxattrErrorPrints();
void getxattrErrorPrints();
