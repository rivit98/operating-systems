#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


void find_files(const char* dir_name, int depth){
	if(depth != -1 && depth == 0){
		return;
	}

	int waitpid_status, status;
	pid_t child_pid = fork();
	if(child_pid == -1){
		printf("fork failed\n");
		exit(1);
	}else if(child_pid == 0){
		printf("PID: %d | PATH: %s\n", getpid(), dir_name);
		status = execlp("ls", "ls", "-l", dir_name, NULL);
		if(status != -1){
			printf("execlp failed\n");
		}
		exit(0);
	}else{
		waitpid(child_pid, &status, 0);
	}

	struct dirent* dir_entry;
	DIR* dir = opendir(dir_name);
	if (dir == NULL) {
		printf("Cannot open directory [%s]\n", dir_name);
		return;
	}

    struct stat lstat_buffer;
	const char* fname;
	while ((dir_entry = readdir(dir)) != NULL) {
		char fname[strlen(dir_name)+strlen(dir_entry->d_name)+2];
		sprintf(fname, "%s/%s", dir_name, dir_entry->d_name);

		if(lstat(fname, &lstat_buffer) == -1){
			printf("Cannot lstat [%s]\n", fname);
			continue;
		}

		if(!S_ISDIR(lstat_buffer.st_mode)){
			continue;
		}

		if(strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0){
			continue;
		}

		find_files(fname, depth-1);
	}

	closedir(dir);
}


int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage:\n");
		printf("find dir [-maxdepth n]\n");
		return 1;
	}

	const char* dir = argv[1];
	int option_maxdepth = -1;

	if(argc > 2){
		char* cmd = argv[2];
		if(strcmp(cmd, "-maxdepth") == 0){
			if(3 >= argc){
				printf("No argument for option -maxdepth\n");
				return 0;
			}

			option_maxdepth = atoi(argv[3]);
			if(option_maxdepth < 0){
				printf("maxdepth cannot be less than 0\n");
				return 0;
			}
		}else{
			printf("Bad argument %s\n", cmd);
			return 0;
		}
	}

	find_files(dir, option_maxdepth);

	return 0;
}
