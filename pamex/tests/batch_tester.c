/**
 * This is the batch tester script to test multiple files in the
 * Pamx parser
 * Author: Daniel Weninger
 * Last Modified: 2/18/2023
*/

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
		printf("Starting Test %d...\n", i);
		char * command = malloc(200);
		sprintf(command, "../parser/parser %s%s%d ../example_data/policy-out ../example_data/leveldb -p", dir_path, "test", i);
		int status = system(command);
		printf("Finished Test %d\n\n", i);
	}
}
