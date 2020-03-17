#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "find_lib.h"


int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage:\n");
		printf("find dir [-mtime n] [-atime n] [-maxdepth n]\n");
		printf("+n for greater than n\n");
		printf("-n for less than n\n");
		printf("n for exactly n\n");
		return 1;
	}

	const char* dir = argv[1];

	Options op;
	memset(&op, 0, sizeof(Options));
	char* cmd;
	for(int i = 2; i < argc; i++){
		cmd = argv[i];
		if(strcmp(cmd, "-mtime") == 0){
			if(i+1 >= argc){
				printf("No argument for option -mtime\n");
				return 0;
			}
			if(op.options_enabled & (1 << MTIME)){
				printf("mtime already specified\n");
				return 0;
			}

			i++;
			op.options_enabled |= (1 << MTIME);
			op.mtime = EXACT;
			if(argv[i][0] == '+'){
				op.mtime = AFTER;
				op.mtime_value = atoi(argv[i]+1);
			}else if(argv[i][0] == '-'){
				op.mtime = BEFORE;
				op.mtime_value = atoi(argv[i]+1);
			}else{
				op.mtime_value = atoi(argv[i]);
			}
		}
		else if(strcmp(cmd, "-atime") == 0){
			if(i+1 >= argc){
				printf("No argument for option -atime\n");
				return 0;
			}
			if(op.options_enabled & (1 << ATIME)){
				printf("atime already specified\n");
				return 0;
			}

			i++;
			op.options_enabled |= (1 << ATIME);
			op.atime = EXACT;
			if(argv[i][0] == '+'){
				op.atime = AFTER;
				op.atime_value = atoi(argv[i]+1);
			}else if(argv[i][0] == '-'){
				op.atime = BEFORE;
				op.atime_value = atoi(argv[i]+1);
			}else{
				op.atime_value = atoi(argv[i]);
			}

		}else if(strcmp(cmd, "-maxdepth") == 0){
			if(i+1 >= argc){
				printf("No argument for option -maxdepth\n");
				return 0;
			}
			if(op.options_enabled & (1 << MAXDEPTH)){
				printf("maxdepth already specified\n");
				return 0;
			}

			i++;
			op.options_enabled |= (1 << MAXDEPTH);
			op.maxdepth = atoi(argv[i]);
			if(op.maxdepth < 0){
				printf("maxdepth cannot be less than 0\n");
				return 0;
			}
		}else{
			printf("Bad argument %s\n", cmd);
			return 0;
		}
	}

	find_files(dir, &op);

	return 0;
}
