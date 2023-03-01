#include <stdlib.h>
#include <stdio.h>

char * getUserFromDB(char * username, FILE * targetedUsersDBFile);
void getxattrErrorPrints();
void makedir(char * path);
void removedirs(char * baseDir);
void createprocfile(int cpid, char * filepath, char * userInfo);
