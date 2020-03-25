#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

const char *data_folder = "data";
const char *out_folder = "out";

int rand_inclusive(int min, int max){
	return rand() % (max + 1 - min) + min;
}

int save_to_file(int rows,int cols, int number){
	char file_name[256];
	sprintf(file_name, "./%s/matrix_in%d.txt", data_folder, number);
	FILE *fp = fopen(file_name, "wt");
	if(!fp){
		printf("Error during opening file %s\n", file_name);
		return 0;
	}

	for(int i = 0; i < rows; i++){
		for(int j = 0; j < cols; j++){
			fprintf(fp, "%d ", rand_inclusive(-100, 100));
		}
		fputs("\n", fp);
	}

	fflush(fp);
	fclose(fp);
	return 1;
}

int main(int argc, char **argv) {
	if (argc != 4) {
		printf("Usage:\n");
		printf("generator min_size max_size num\n");
		return 1;
	}

	int min_size = atoi(argv[1]);
	int max_size = atoi(argv[2]);
	int num = atoi(argv[3]);

	if(min_size <= 0 || max_size <= 0 || num <= 0 || min_size >= max_size){
		printf("Please specify valid number\n");
		return 0;
	}

	char data_f[64];
	sprintf(&data_f[0], "./%s", data_folder);
	int res = mkdir(data_f, 0777);
	if(res == -1){
		if(errno == EEXIST){

		}else{
			printf("Error during creating %s folder\n", data_f);
			exit(EXIT_FAILURE);
		}
	}

	FILE *fp = fopen("./lista.txt", "wt");
	if(!fp){
		printf("Error during opening file ./lista.txt\n");
		return 1;
	}
	
	srand(time(NULL));
	int filecounter = 0;
	int cnt = 0;
	int orig_num = num;
	while(num-- > 0){
		int cols = rand_inclusive(min_size, max_size);
		int rows = rand_inclusive(min_size, max_size);
		int cols2 = rand_inclusive(min_size, max_size);

		if(save_to_file(rows, cols, filecounter) && save_to_file(cols, cols2, filecounter+1)){
			fprintf(fp, "./%s/matrix_in%d.txt ./%s/matrix_in%d.txt matrix_%d%d.txt\n",
						data_folder, filecounter, data_folder, filecounter+1, filecounter, filecounter+1);
			
			filecounter += 2;
			cnt++;
		}
	}

	printf("%d/%d par stworzono pomyslnie!\n", cnt, orig_num);

	fflush(fp);
	fclose(fp);

	return 0;
}