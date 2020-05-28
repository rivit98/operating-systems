#include "io_lib.h"

void generate_records(const char* filename, int records, int record_size){
	if(records <= 0){
		printf("records_num should be greater than 0\n");
		return;
	}

	if(record_size <= 0){
		printf("record_size should be greater than 0\n");
		return;
	}

	char command[256];
	snprintf(command, sizeof command, "cat /dev/urandom | tr -dc 'a-z0-9' | fold -w %d | head -n %d > %s", record_size, records, filename);
	int ret = system(command);
	if(ret != 0){
		printf("Error during generating file\n");
		return;
	}
}

void sort_records(const char* filename, int records, int record_size, const char* type){
	if(records <= 0){
		printf("records_num should be greater than 0\n");
		return;
	}

	if(record_size <= 0){
		printf("record_size should be greater than 0\n");
		return;
	}

	if(strcmp(type, "sys") == 0){
		int f = open(filename, O_RDWR);
		if(f == -1){
			printf("Cannot open file [%s]\n", filename);
			return;
		}
		quick_sort_sys(f, 0, records-1, record_size+1);
		close(f);

	}else if(strcmp(type, "lib") == 0){
		FILE* f = fopen(filename, "r+");
		if(f == NULL){
			printf("Cannot open file [%s]\n", filename);
			return;
		}
		quick_sort_lib(f, 0, records-1, record_size+1); //+1 because of new line
		fclose(f);
	}else{
		printf("please specify valid type (lib/sys)\n");
		return;
	}
}

void quick_sort_lib(FILE* f, int l, int h, int record_size){
	if(l < h){
		int pi = partition_lib(f, l, h, record_size);
		quick_sort_lib(f, l, pi-1, record_size);
		quick_sort_lib(f, pi+1, h, record_size);
	}
}

int partition_lib(FILE* f, int l, int h, int record_size){
	char* pivot = calloc(record_size, sizeof(char));
	char* current = calloc(record_size, sizeof(char));
	read_line_lib(f, h, pivot, record_size);
	int i = l - 1;

	for(int j = l; j < h; j++){
		read_line_lib(f, j, current, record_size);
		if(strcmp(current, pivot) < 0){
			i++;
			swap_in_file_lib(f, i, j, record_size);
		}
	}
	i++;

	free(pivot);
	free(current);

	swap_in_file_lib(f, i, h, record_size);
	return i;
}

void read_line_lib(FILE* f, int line, char* buffer, int size){
	fseek(f, line*size, 0);
	fread(buffer, sizeof(char), size, f);
}

void save_line_lib(FILE* f, int line, char* buffer, int size){
	fseek(f, line*size, 0);
	fwrite(buffer, sizeof(char), size, f);
}

void swap_in_file_lib(FILE* f, int i, int j, int size){
	char* a = calloc(size, sizeof(char));
	char* b = calloc(size, sizeof(char));

	read_line_lib(f, i, a, size);
	read_line_lib(f, j, b, size);

	save_line_lib(f, i, b, size);
	save_line_lib(f, j, a, size);

	free(a);
	free(b);
}

void quick_sort_sys(int f, int l, int h, int record_size){
	if(l < h){
		int pi = partition_sys(f, l, h, record_size);
		quick_sort_sys(f, l, pi-1, record_size);
		quick_sort_sys(f, pi+1, h, record_size);
	}
}

int partition_sys(int f, int l, int h, int record_size){
	char* pivot = calloc(record_size, sizeof(char));
	char* current = calloc(record_size, sizeof(char));
	read_line_sys(f, h, pivot, record_size);
	int i = l - 1;

	for(int j = l; j < h; j++){
		read_line_sys(f, j, current, record_size);
		if(strcmp(current, pivot) < 0){
			i++;
			swap_in_file_sys(f, i, j, record_size);
		}
	}
	i++;

	free(pivot);
	free(current);

	swap_in_file_sys(f, i, h, record_size);

	return i;
}

void read_line_sys(int f, int line, char* buffer, int size){
	lseek(f, line*size, 0);
	read(f, buffer, size);
}

void save_line_sys(int f, int line, char* buffer, int size){
	lseek(f, line*size, 0);
	write(f, buffer, size);
}

void swap_in_file_sys(int f, int i, int j, int size){
	char* a = calloc(size, sizeof(char));
	char* b = calloc(size, sizeof(char));

	read_line_sys(f, i, a, size);
	read_line_sys(f, j, b, size);

	save_line_sys(f, i, b, size);
	save_line_sys(f, j, a, size);

	free(a);
	free(b);
}

void copy_records_lib(const char* filename, const char* copy, int records, int record_size){
	FILE* fr = fopen(filename, "r");
	if(fr == NULL){
		printf("Error durin opening file [%s]", filename);
		return;
	}
	FILE* fw = fopen(copy, "w");
	if(fw == NULL){
		printf("Error durin opening file [%s]", copy);
		return;
	}

	char* buffer = calloc(record_size+1, sizeof(char));
	for(int i = 0; i < records; i++){
		fread(buffer, sizeof(char), record_size+1, fr);
		fwrite(buffer, sizeof(char), record_size+1, fw);
	}

	free(buffer);
	fclose(fr);
	fclose(fw);
}

void copy_records_sys(const char* filename, const char* copy, int records, int record_size){
	int fr = open(filename, O_RDONLY);
	if(fr == -1){
		printf("Error durin opening file [%s]", filename);
		return;
	}
	int fw = open(copy, O_WRONLY | O_CREAT | O_TRUNC);
	if(fw == -1){
		printf("Error durin opening file [%s]", copy);
		return;
	}

	char* buffer = calloc(record_size+1, sizeof(char));
	for(int i = 0; i < records; i++){
		read(fr, buffer, record_size+1);
		write(fw, buffer, record_size+1);
	}

	free(buffer);
	close(fr);
	close(fw);
}

void copy_records(const char* filename, const char* copy, int records, int record_size, const char* type){
	if(records <= 0){
		printf("records_num should be greater than 0\n");
		return;
	}

	if(record_size <= 0){
		printf("record_size should be greater than 0\n");
		return;
	}

	if(strcmp(type, "sys") == 0){
		copy_records_sys(filename, copy, records, record_size);
	}else if(strcmp(type, "lib") == 0){
		copy_records_lib(filename, copy, records, record_size);
	}else{
		printf("please specify valid type (lib/sys)\n");
		return;
	}
}