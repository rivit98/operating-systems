#pragma once
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <ftw.h>


enum {
	MTIME = 1,
	ATIME,
	MAXDEPTH
};

typedef enum {
	BEFORE = -1,
	EXACT = 0,
	AFTER = 1
} MODE;

typedef struct options{
	unsigned short options_enabled;
	MODE mtime;
	unsigned int mtime_value;
	MODE atime;
	unsigned int atime_value;
	unsigned int maxdepth;

} Options;

void find_files(const char* dir, const Options* op);
void find_files_nftw(const char* dir, const Options* op);