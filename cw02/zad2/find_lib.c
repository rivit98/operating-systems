#include "find_lib.h"


const char* get_file_type(mode_t m){
	if (S_ISREG(m)) return "file";
	if (S_ISFIFO(m)) return "fifo";
	if (S_ISDIR(m)) return "dir";
	if (S_ISCHR(m)) return "char dev";
	if (S_ISLNK(m)) return "slink";
	if (S_ISBLK(m)) return "block dev";
	if (S_ISSOCK(m)) return "sock";
	return "unknown";
}

void print_file_info(const char* dir_name, const Options* op, const struct stat* lstat_buffer){
	char buf[PATH_MAX];
	char* res = realpath(dir_name, buf); //get abs path
	if(res && !S_ISLNK(lstat_buffer->st_mode)){
		printf("%-100s  ", buf);
	}else{
		printf("%-100s  ", dir_name);
	}
	printf("%-10s ", get_file_type(lstat_buffer->st_mode));
	printf("%10ld bytes  ", lstat_buffer->st_size);
	printf("links: %-3ld  ", lstat_buffer->st_nlink);

	strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&lstat_buffer->st_atime));
	printf("a: %-20s  ", buf);

	strftime(buf, 20, "%Y-%m-%d %H:%M:%S", localtime(&lstat_buffer->st_mtime));
	printf("m: %-20s\n", buf);
}

int check_conditions(const char* filename, const Options* op, const struct stat* lstat_buffer){
	char ret = 1;
	time_t current_time = time(NULL);
	long int diff = 0;
	if(op->options_enabled & (1 << ATIME)){
		diff = (int)difftime(current_time, lstat_buffer->st_atime);
		diff /= (60 * 60 * 24);

		switch(op->atime){
			case AFTER:
				ret = (diff > op->atime_value);
				break;

			case EXACT:
				ret = (diff == op->atime_value);
				break;

			case BEFORE:
				ret = (diff < op->atime_value);
				break;
		}
	}

	if(op->options_enabled & (1 << MTIME)){
		diff = (int)difftime(current_time, lstat_buffer->st_mtime);
		diff /= (60 * 60 * 24);

		switch(op->mtime){
			case AFTER:
				ret = (diff > op->mtime_value);
				break;

			case EXACT:
				ret = (diff == op->mtime_value);
				break;

			case BEFORE:
				ret = (diff < op->mtime_value);
				break;
		}
	}

	return (int)ret;
}

void find_files_rec(const char* dir_name, const Options* op, struct stat* lstat_buffer, int depth){
	if((op->options_enabled & (1 << MAXDEPTH) && depth <= 0)){
		return;
	}
	struct dirent* dir_entry;
	DIR* dir = opendir(dir_name);
	if (dir == NULL) {
		printf("Cannot open directory [%s]\n", dir_name);
		return;
	}

	const char* fname;
	while ((dir_entry = readdir(dir)) != NULL) {
		char fname[strlen(dir_name)+strlen(dir_entry->d_name)+2];
		sprintf(fname, "%s/%s", dir_name, dir_entry->d_name);

		if(strcmp(dir_entry->d_name, "..") == 0 || strcmp(dir_entry->d_name, ".") == 0){
			continue;
		}

		if(lstat(fname, lstat_buffer) < 0){
			printf("Cannot lstat %s\n", fname);
			continue;
		}

		if(S_ISDIR(lstat_buffer->st_mode)){
			if((op->options_enabled & (1 << MAXDEPTH) && depth > 0) || !(op->options_enabled & (1 << MAXDEPTH))){
				find_files_rec(fname, op, lstat_buffer, depth-1);
			}
		}
		// }else{
			if(check_conditions(fname, op, lstat_buffer)){
				print_file_info(fname, op, lstat_buffer);
			}
		// }
	}

	closedir(dir);
}

void find_files(const char* dir_name, const Options* op){
	struct stat lstat_buffer; //avoid to creating many structs
	find_files_rec(dir_name, op, &lstat_buffer, op->maxdepth);
}

///////////////////// NTFW VERSION

const Options* ntfw_options;

static int display_info(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuf)
{
	if ((ntfw_options->options_enabled & (1 << MAXDEPTH)) && ftwbuf->level > ntfw_options->maxdepth) {
		return 0; 
	}

	// if(tflag == FTW_D){
	// 	return 0;
	// }

	if(check_conditions(fpath, ntfw_options, sb)){
		print_file_info(fpath, ntfw_options, sb);
	}

	return 0;
}

void find_files_nftw(const char* dir_name, const Options* op){
	ntfw_options = op;
	nftw(dir_name, display_info, 20, FTW_PHYS);
}