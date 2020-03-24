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
const char *temp_folder = "tmp";

typedef struct{
	const char *list_filename;
	unsigned int proc_num;
	unsigned int proc_time;
	unsigned int matrix_pairs;
} Options;

typedef struct{
	int **data;
	unsigned int cols;
	unsigned int rows;
} Matrix;

typedef struct{
	Matrix* A_matrices;
	Matrix* B_matrices;
	char **output_files;
	unsigned int size;
} List;


void load_pairs(List *list, const char *list_filename);
void load_matrices(char *list_filename, Matrix *A, Matrix *B, char **out_filename);
void load_matrix(Matrix *matrix, char *file_name);
unsigned int get_matrix_rows(FILE *fp);
unsigned int get_matrix_columns(FILE *fp);
char *rtrim(char *s);
void free_matrix(Matrix *A);
void dump_matrix(Matrix *A);
void create_marker_file(char *out_filename, int cols);
unsigned int worker_cb(List *list, time_t deadline);
void dump_matrix(Matrix *matrix);
void multiply_column(const char *out_filename, Matrix *A, Matrix *B, int col_idx);
void concat_parts(char* out_filename);
int get_not_calculated_column(char *out_filename);


void free_list(List *list){
	for(int i = 0; i < list->size; i++){
		free(list->output_files[i]);
		free_matrix(&list->A_matrices[i]);
		free_matrix(&list->B_matrices[i]);
	}
	free(list->A_matrices);
	free(list->B_matrices);
	free(list->output_files);
}

void free_matrix(Matrix *matrix){
	for(int i = 0; i < matrix->rows; i++){
		free(matrix->data[i]);
	}
	free(matrix->data);
}

void dump_matrix(Matrix *matrix){
	printf("rows %d | cols %d | data %p\n", matrix->rows, matrix->cols, matrix->data);
	for(int i = 0; i < matrix->rows; i++){
		for(int j = 0; j < matrix->cols; j++){
			printf("%4d ", matrix->data[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void load_pairs(List *list, const char *list_filename){
	FILE *fp = fopen(list_filename, "rt");
	if(!fp){
		printf("Error during opening file %s\n", list_filename);
		exit(EXIT_FAILURE);
	}

	int i = 0;
	char buffer[256]; //just hardcode line len
	while(fgets(buffer, sizeof(buffer)-1, fp) != NULL && i < list->size) {
		if(strlen(buffer) < 5){
			list->size--;
			continue;
		}
		load_matrices(&buffer[0], &list->A_matrices[i], &list->B_matrices[i], &list->output_files[i]);
		create_marker_file(list->output_files[i], list->B_matrices[i].cols);
		i++;
	}

	fclose(fp);
}

void load_matrices(char *buffer, Matrix *A, Matrix *B, char **out_filename){
	const char *token = " ";
	char *m1 = strtok(buffer, token);
	char *m2 = strtok(NULL, token);
	*out_filename = strdup(strtok(NULL, token));
	rtrim(*out_filename);

	load_matrix(A, m1);
	load_matrix(B, m2);

	if(A->cols != B->rows){
		printf("Wrong matrices dimensions - cannot multiply!\n");
		exit(EXIT_FAILURE);
	}
}

void load_matrix(Matrix *matrix, char *file_name){
	FILE *fp = fopen(file_name, "rt");
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
	const char *token = " ";
	char *part;
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

unsigned int get_number_of_lines(const char *file_name){
	FILE *fp = fopen(file_name, "rt");
	if(!fp){
		printf("Error during opening file %s\n", file_name);
		exit(EXIT_FAILURE);
	}
	unsigned int ret = get_matrix_rows(fp);
	fclose(fp);
	return ret;
}

unsigned int get_matrix_rows(FILE *fp){
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

unsigned int get_matrix_columns(FILE *fp){
	rewind(fp);
	unsigned int ret = 0;
	char buf[MAX_LINE_LEN];
	fgets(buf, sizeof(buf)-1, fp);
	rtrim(buf);
	char *c = &buf[0];
	while(*c != '\0'){
		if(*c == ' '){
			ret++;
		}
		c++;
	}
	return ret + 1;
}

char *rtrim(char *s){
	char *back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

void create_marker_file(char *out_filename, int cols){
	int res = mkdir("./tmp", 0777);
	if(res == -1){
		if(errno == EEXIST){

		}else{
			printf("Error during creating tmp folder\n");
			exit(EXIT_FAILURE);
		}
	}
	char filename[256];
	sprintf(filename, "./%s/%s-marker.txt", temp_folder, out_filename);
	FILE *fp = fopen(filename, "w+");
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

int get_not_calculated_column(char *out_filename){
	char filename[256];
	sprintf(filename, "./%s/%s-marker.txt", temp_folder, out_filename);
	FILE *fp = fopen(filename, "r+");
	if(!fp){
		printf("Error during opening %s\n", filename);
		exit(EXIT_FAILURE);
	}

	int fno = fileno(fp);
	flock(fno, LOCK_EX);

	int index = 0, c = 0;
	while ((c = fgetc(fp)) != EOF) {
		if(c == '0'){
			fseek(fp, index, SEEK_SET);
			fputc('1', fp);
			break;
		}else{
			index++;
		}
	}
	fflush(fp);
	flock(fno, LOCK_UN);
	fclose(fp);
	return (c == EOF) ? -1 : index;
}

void multiply_column(const char *out_filename, Matrix *A, Matrix *B, int col_idx){
	char filename[256];
	sprintf(filename, "./%s/%s_%d.txt", temp_folder, out_filename, col_idx);

	FILE *fp = fopen(filename, "w+");
	if(!fp){
		printf("Error during creating %s\n", filename);
		exit(EXIT_FAILURE);
	}

	int sum = 0;
	for(int row = 0; row < A->rows; row++){
		for(int col = 0; col < A->cols; col++){
			sum += A->data[row][col] * B->data[col][col_idx];
		}
		fprintf(fp, "%d \n", sum);
		sum = 0;
	}

	fflush(fp);
	fclose(fp);
}

unsigned int worker_cb(List *list, time_t deadline){
	unsigned int ret = 0;
	int col_to_mult;

	for(int i = 0; i < list->size; i++){
		while((col_to_mult = get_not_calculated_column(list->output_files[i])) != -1){
			if(time(NULL) > deadline){
				printf("Time exceeded\n");
				break;
			}

			multiply_column(list->output_files[i], &list->A_matrices[i], &list->B_matrices[i], col_to_mult);
			ret++;
		}
	}

	return ret;
}

void concat_parts(char* out_filename){
	char command[1024];
	sprintf(command, "paste ./%s/%s_* > ./out/%s", temp_folder, out_filename, out_filename);
	system(command);
}

int main(int argc, char **argv) {
	if (argc != 4) {
		printf("Usage:\n");
		printf("macierz lista proc_num time\n");
		return 1;
	}

	Options op;
	op.list_filename = argv[1];
	op.proc_num = atoi(argv[2]);
	op.proc_time = atoi(argv[3]);
	op.matrix_pairs = get_number_of_lines(op.list_filename);

	if(op.proc_num <= 0 || op.proc_time <= 0 || op.matrix_pairs <= 0){
		printf("Please specify valid number\n");
		return 0;
	}

	List m_list;
	m_list.size = op.matrix_pairs;
	m_list.A_matrices = calloc(m_list.size, sizeof(Matrix));
	m_list.B_matrices = calloc(m_list.size, sizeof(Matrix));
	m_list.output_files = calloc(m_list.size, sizeof(char*));

	load_pairs(&m_list, op.list_filename);

	// for(int i = 0; i < m_list.size; i++){
	// 	printf("Saving to: %s\n", m_list.output_files[i]);
	// 	dump_matrix(&m_list.A_matrices[i]);
	// 	dump_matrix(&m_list.B_matrices[i]);
	// }
	
	time_t cur_time = time(NULL);
	pid_t *workers = calloc(op.proc_num, sizeof(pid_t));
	for(int i = 0; i < op.proc_num; i++){
		pid_t child = fork();
		if(child == 0){
			int mul = worker_cb(&m_list, cur_time + op.proc_time);
			exit(mul);
		}else if(child > 0){
			workers[i] = child;
		}else{
			printf("Something went wrong during fork\n");
			exit(EXIT_FAILURE);
		}
	}

	int ret_status;
	for(int i = 0; i < op.proc_num; i++){
		waitpid(workers[i], &ret_status, 0);
		printf("Proces %d wykonal %d mnozen macierzy\n", workers[i], WEXITSTATUS(ret_status));
	}
	
	int res = mkdir("./out", 0777);
	if(res == -1){
		if(errno == EEXIST){

		}else{
			printf("Error during creating out folder\n");
			exit(EXIT_FAILURE);
		}
	}
	for(int i = 0; i < m_list.size; i++){
		concat_parts(m_list.output_files[i]);
	}
	free(workers);
	free_list(&m_list);

	return 0;
}


