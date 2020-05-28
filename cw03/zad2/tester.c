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
const char *out_folder = "out";

typedef struct{
	const char *list_filename;
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
void multiply_matrices(Matrix *a, Matrix *b, Matrix *c);
int verify_results(Matrix *m, char *out_filename);
void save_to_file(Matrix *m, char *f);


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
	char buffer[MAX_LINE_LEN];
	while(fgets(buffer, sizeof(buffer)-1, fp) != NULL && i < list->size) {
		load_matrices(&buffer[0], &list->A_matrices[i], &list->B_matrices[i], &list->output_files[i]);
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
	unsigned int ret = 0;
	char buffer[MAX_LINE_LEN];
	while(fgets(&buffer[0], sizeof(buffer)-1, fp) != NULL) {
		if(strlen(buffer) < 5){
			continue;
		}
		ret++;
	}
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

void multiply_matrices(Matrix *a, Matrix *b, Matrix *c){
	c->rows = a->rows;
	c->cols = b->cols;
	c->data = calloc(c->rows, sizeof(int *));
	for(int i = 0; i < c->rows; i++){
		c->data[i] = calloc(c->cols, sizeof(int));
	}

	int sum = 0;
	for(int i = 0; i < a->rows; i++){
		for(int j = 0; j < b->cols; j++){
			sum = 0;
			for(int k = 0; k < a->cols; k++){
				sum += a->data[i][k] * b->data[k][j];
			}
			c->data[i][j] = sum;
		}
	}
}

int load_matrix_fast(Matrix *matrix, char *f){
	char file_name[MAX_LINE_LEN];
	sprintf(file_name, "./out/%s", f);
	FILE *fp = fopen(file_name, "rt");
	if(!fp){
		printf("Error during opening file %s\n", file_name);
		return 1;
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
			return 1;
		}
		part = strtok(buf, token);
		while(part != NULL){
			if(col_indexer >= matrix->cols){
				return 1;
			}

			matrix->data[row_indexer][col_indexer++] = atoi(part);
			part = strtok(NULL, token);
		}

		row_indexer++;
		col_indexer = 0;
		part = NULL;
	}

	fclose(fp);
	return 0;
}

int compare_matrices(Matrix *m, Matrix *m2){
	for(int i = 0; i < m->rows; i++){
		for(int j = 0; j < m->cols; j++){
			if(m->data[i][j] != m2->data[i][j]){
				printf("%d %d | %d vs %d\n", i, j, m->data[i][j], m2->data[i][j]);
				return 0;
			}
		}
	}

	return 1;
}

int verify_results(Matrix *m, char *out_filename){
	Matrix m2;
	m2.cols = m->cols;
	m2.rows = m->rows;
	m2.data = calloc(m2.rows, sizeof(int *));
	for(int i = 0; i < m2.rows; i++){
		m2.data[i] = calloc(m2.cols, sizeof(int));
	}

	int ret = load_matrix_fast(&m2, out_filename);
	if(ret != 0){
		free_matrix(&m2);
		return ret;
	}

	ret = compare_matrices(m, &m2);
	// save_to_file(m, out_filename);

	free_matrix(&m2);
	return ret;
}

void save_to_file(Matrix *m, char *f){
	char file_name[256];
	sprintf(file_name, "./%s/%s_expected.txt", out_folder, f);
	FILE *fp = fopen(file_name, "wt");
	if(!fp){
		printf("Error during opening file %s\n", file_name);
		return;
	}

	for(int i = 0; i < m->rows; i++){
		for(int j = 0; j < m->cols; j++){
			fprintf(fp, "%d ", m->data[i][j]);
		}
		fputs("\n", fp);
	}

	fflush(fp);
	fclose(fp);
}


int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage:\n");
		printf("tester lista_macierzy\n");
		return 1;
	}

	Options op;
	op.list_filename = argv[1];
	op.matrix_pairs = get_number_of_lines(op.list_filename);

	if(op.matrix_pairs <= 0){
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

	int cnt = 0;
	for(int i = 0; i < m_list.size; i++){
		Matrix m3;
		multiply_matrices(&m_list.A_matrices[i], &m_list.B_matrices[i], &m3);
		// dump_matrix(&m3);
		if(!verify_results(&m3, m_list.output_files[i])){
			printf("%s -> Results don't match\n", m_list.output_files[i]);
		}else{
			cnt++;
		}
		free_matrix(&m3);
	}

	printf("%d/%d testow pomyslnych!\n", cnt, m_list.size);

	free_list(&m_list);

	return 0;
}


