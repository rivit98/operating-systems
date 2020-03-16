#pragma once

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void generate_records(const char* filename, int records, int record_size);
void sort_records(const char* filename, int records, int record_size, const char* type);
void copy_records(const char* filename, const char* copy, int records, int record_size, const char* type);

void quick_sort_lib(FILE* f, int l, int h, int record_size);
int partition_lib(FILE* f, int l, int h, int record_size);
void read_line_lib(FILE* f, int line, char* buffer, int size);
void swap_in_file_lib(FILE* f, int i, int j, int size);
void save_line_lib(FILE* f, int line, char* buffer, int size);
void copy_records_lib(const char* filename, const char* copy, int records, int record_size);

void quick_sort_sys(int f, int l, int h, int record_size);
int partition_sys(int f, int l, int h, int record_size);
void read_line_sys(int f, int line, char* buffer, int size);
void swap_in_file_sys(int f, int i, int j, int size);
void save_line_sys(int f, int line, char* buffer, int size);
void copy_records_sys(const char* filename, const char* copy, int records, int record_size);
