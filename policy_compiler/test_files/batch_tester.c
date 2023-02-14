#include <stdlib.h>
#include <stdio.h>

int main(int argc, char ** argv) {
	int consecutive_test_files = 11;
	char * dir_path = "./";
	if(argc > 1) {
		dir_path = argv[1];
	}	
	if(argc == 3) {
		consecutive_test_files = atoi(argv[2]);
	}
	for(int i = 1; i <= consecutive_test_files; i++) {
		char * command = malloc(200);
		sprintf(command, "../parser %s%s%d -p", dir_path, "test", i);
		int status = system(command);
	}
}
