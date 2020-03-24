#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/file.h>


const int MAX_LINE_LEN = 2048;
const char* temp_folder = "tmp";

typedef struct{
	const char* list_filename;
	char* out_filename;
	int proc_num;
	int proc_time;
} Options;

typedef struct{
	int** data;
	unsigned int cols;
	unsigned int rows;
} Matrix;



void load_matrices(Matrix* A, Matrix* B, Options* op);
void load_matrix(Matrix* matrix, char* file_name);
unsigned int get_matrix_rows(FILE* fp);
unsigned int get_matrix_columns(FILE* fp);
char* rtrim(char *s);
void free_matrix(Matrix* A);
void dump_matrix(Matrix* A);
void create_marker_file(char* out_filename, int cols);
unsigned int worker_cb(const char* out_filename, Matrix* A, Matrix* B, time_t deadline);
void dump_matrix(Matrix* matrix);


void free_matrix(Matrix* matrix){
	for(int i = 0; i < matrix->rows; i++){
		free(matrix->data[i]);
	}

	free(matrix->data);
	matrix = NULL;
}

void dump_matrix(Matrix* matrix){
	printf("\nrows %d | cols %d | data %p\n", matrix->rows, matrix->cols, matrix->data);
	for(int i = 0; i < matrix->rows; i++){
		for(int j = 0; j < matrix->cols; j++){
			printf("%4d ", matrix->data[i][j]);
		}
		printf("\n");
	}
}

void load_matrices(Matrix* A, Matrix* B, Options* op){
	FILE* fp = fopen(op->list_filename, "rt");
	if(!fp){
		printf("Error during opening file %s\n", op->list_filename);
		exit(EXIT_FAILURE);
	}

	char buffer[256]; //just hardcode line len
	fread(&buffer, sizeof(char), sizeof(buffer) - 1, fp);
	fclose(fp);

	const char* token = " ";
	char* m1 = strtok(buffer, token);
	char* m2 = strtok(NULL, token);
	op->out_filename = strdup(strtok(NULL, token));

	load_matrix(A, m1);
	load_matrix(B, m2);
}

void load_matrix(Matrix* matrix, char* file_name){
	FILE* fp = fopen(file_name, "rt");
	if(!fp){
		printf("Error during opening file %s\n", file_name);
		exit(EXIT_FAILURE);
	}

	matrix->rows = get_matrix_rows(fp);
	matrix->cols = get_matrix_columns(fp);
	matrix->data = calloc(matrix->rows, sizeof(int*));
	for(int i = 0; i < matrix->rows; i++){
		matrix->data[i] = calloc(matrix->cols, sizeof(int));
	}
	
	int row_indexer = 0;
	int col_indexer = 0;
	const char* token = " ";
	char* part;
	char buf[MAX_LINE_LEN];
	rewind(fp);
	while(fgets(buf, sizeof(buf)-1, fp) != NULL){
		rtrim(buf);
		if(row_indexer >= matrix->rows){
			printf("Too many rows for %s!\n", file_name);
			exit(EXIT_FAILURE);
		}
		part = strtok(buf, token);
		while(part != NULL){
			if(col_indexer >= matrix->cols){
				printf("Too many columns for %s!\n", file_name);
				exit(EXIT_FAILURE);
			}

			matrix->data[row_indexer][col_indexer++] = atoi(part);
			part = strtok(NULL, token);
		}

		row_indexer++;
		col_indexer = 0;
		part = NULL;
	}

	fclose(fp);
}

unsigned int get_matrix_rows(FILE* fp){
	rewind(fp);
	unsigned int ret = 0;
	char c, last;
	while((c = getc(fp)) != EOF){
		if(c == '\n'){
			ret++;
		}
		last = c;
	}
	if(last != '\n'){
		ret++; //Count the last line even if it lacks a newline
	}

	return ret;
}

char* rtrim(char *s){
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

unsigned int get_matrix_columns(FILE* fp){
	rewind(fp);
	unsigned int ret = 0;
	char buf[MAX_LINE_LEN];
	fgets(buf, sizeof(buf)-1, fp);
	rtrim(buf);
	char* c = &buf[0];
	while(*c != '\0'){
		if(*c == ' '){
			ret++;
		}
		c++;
	}
	return ret + 1;
}

void create_marker_file(char* out_filename, int cols){
	int res = mkdir("./tmp", 0777);
	if(res == -1){
		if(errno == EEXIST){

		}else{
			printf("Error during creating tmp folder\n");
			exit(EXIT_FAILURE);
		}
	}
	char filename[256];
	sprintf(filename, "./%s/%s_marker.txt", temp_folder, out_filename);
	FILE* fp = fopen(filename, "w+");
	if(!fp){
		printf("Error during creating %s\n", filename);
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < cols; i++){
		fwrite("0", sizeof(char), 1, fp);
	}

	fflush(fp);
	fclose(fp);
}

int get_not_calculated_column(const char* out_filename){
	char filename[256];
	sprintf(filename, "./%s/%s_marker.txt", temp_folder, out_filename);
	FILE* fp = fopen(filename, "r+");
	if(!fp){
		printf("Error during creating %s\n", filename);
		exit(EXIT_FAILURE);
	}

	int fno = fileno(fp);
	flock(fno, LOCK_EX);

	int index = 0;
	int c;
	while ((c = fgetc(fp)) != EOF) {
		if(c == '0'){
			fseek(fp, index, SEEK_SET);
			fputc('1', fp);
			fflush(fp);
			break;
		}else{
			index++;
		}
	}

	flock(fno, LOCK_UN);
	fclose(fp);
	return (c == EOF) ? -1 : index;
}

unsigned int worker_cb(const char* out_filename, Matrix* A, Matrix* B, time_t deadline){
	unsigned int ret = 0;

	for(;;){
		// if(time(NULL) > deadline){
		// 	printf("Time exceeded\n");
		// 	break;
		// }

		int col_to_mult = get_not_calculated_column(out_filename);
		if(col_to_mult == -1){
			break;
		}
		printf("Calculating row: %d\n", col_to_mult);
		// sleep(1);
		ret++;
	}


	return ret;
}

int main(int argc, char** argv) {
	if (argc != 4) {
		printf("Usage:\n");
		printf("macierz lista proc_num time\n");
		return 1;
	}

	Options op;
	op.list_filename = argv[1];
	op.proc_num = atoi(argv[2]);
	op.proc_time = atoi(argv[3]);

	if(op.proc_num < 0 || op.proc_time < 0){
		printf("Please specify valid number\n");
		return 0;
	}

	Matrix A, B;
	load_matrices(&A, &B, &op);

	// dump_matrix(&A);
	// dump_matrix(&B);

	time_t cur_time = time(NULL);
	create_marker_file(op.out_filename, B.cols);
	pid_t* workers = calloc(op.proc_num, sizeof(pid_t));
	for(int i = 0; i < op.proc_num; i++){
		pid_t child = fork();
		if(child == 0){
			int mul = worker_cb(op.out_filename, &A, &B, cur_time + op.proc_time);
			exit(mul);
		}else{
			workers[i] = child;
		}
	}

	int ret_status;
	for(int i = 0; i < op.proc_num; i++){
		waitpid(workers[i], &ret_status, 0);
		printf("Proces %d wykonal %d mnozen macierzy\n", workers[i], WEXITSTATUS(ret_status));
	}
	
	//concat parts here

	free(workers);
	free_matrix(&A);
	free_matrix(&B);
	free(op.out_filename);

	return 0;
}


