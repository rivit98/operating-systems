#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <pthread.h>
#include <math.h>

/*
PGMA is a data directory which contains examples of the ASCII PGM (Portable Gray Map) file format. It is a simple grayscale image description. The definition is as follows:

the typical file extension is ".pgm", but an extension of ".pnm" is also occasionally used.
A "magic number" for identifying the file type. An ASCII PGM file's magic number is the two characters "P2".
Whitespace (blanks, TABs, CRs, LFs).
A width, formatted as ASCII characters in decimal.
Whitespace.
A height, again in ASCII decimal.
Whitespace.
The maximum gray value, again in ASCII decimal.
Whitespace.
Width * height gray values, each in ASCII decimal, between 0 and the specified maximum value, separated by whitespace, starting at the top-left corner of the graymap, proceeding in normal English reading order. A value of 0 means black, and the maximum value means white.
Characters from a "#" to the next end-of-line are ignored (comments). No line should be longer than 70 characters.
*/

#define MAX_LINE_LEN 70
#define MAX_COLORS 256
#define For(VAR, START_VALUE, END_VALUE, STEP) for(int VAR = START_VALUE; VAR < END_VALUE; VAR += STEP)
#define ForRange(START_VALUE, END_VALUE) for(int i = START_VALUE; i < END_VALUE; i += 1)

struct {
    int height;
    int width;
    unsigned char **data;
} IMAGE_DATA;

int threads_num;
int histogram[MAX_COLORS] = {0};

int parse_mode(const char *mode){
    if(strcmp(mode, "sign") == 0){
        return 0;
    }else if(strcmp(mode, "block") == 0){
        return 1;
    }else if(strcmp(mode, "interleaved") == 0){
        return 2;
    }

    puts("Wrong mode");
    exit(EXIT_FAILURE);
}

//skips comments and empty lines
void read_skipping_comment(char *buffer, FILE *f){
    do{
        if(fgets(buffer, MAX_LINE_LEN, f) == NULL){
            memset(buffer, 0, MAX_LINE_LEN);
            return;
        }
    }while(buffer[0] == '\n' || buffer[0] == '#');
}

void load_image(const char *filename){
    const char* delimiters = " \n\r\t";
    char buffer[MAX_LINE_LEN];
    FILE *f = fopen(filename, "r");
    if(f == NULL){
        printf("Cannot open file [%s] for read", filename);
        exit(EXIT_FAILURE);
    }

    read_skipping_comment(buffer, f); //P2
    read_skipping_comment(buffer, f); //skip comment, read w and h

    IMAGE_DATA.width = atoi(strtok(buffer, delimiters));
    IMAGE_DATA.height = atoi(strtok(NULL, delimiters));
    IMAGE_DATA.data = calloc(IMAGE_DATA.height, sizeof(unsigned char *));
    ForRange(0, IMAGE_DATA.height){
        IMAGE_DATA.data[i] = calloc(IMAGE_DATA.width, sizeof(unsigned char));
    }

    read_skipping_comment(buffer, f); //skip max gray value

    int i = 0;
    char *value = NULL;
    do{
        value = strtok(NULL, delimiters);
        if(value == NULL){ //no data left in buffer
            read_skipping_comment(buffer, f);
            value = strtok(buffer, delimiters);
        }

        IMAGE_DATA.data[i / IMAGE_DATA.width][i % IMAGE_DATA.width] = atoi(value);
        i++;
    }while(i < IMAGE_DATA.width * IMAGE_DATA.height);

    fclose(f);
}

long get_time(struct timespec *s) {
    struct timespec e;
    clock_gettime(CLOCK_MONOTONIC, &e);
    long retval = (e.tv_sec - s->tv_sec) * 1000000;
    retval += (e.tv_nsec - s->tv_nsec) / 1000.0;
    return retval;
}

long sign_mode_thread(int *tid){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    //we need to split 256 colours into equal parts (or almost equal)
    int start_number;
    int end_number;
    int k = *tid;
    int non_complementary = threads_num - (MAX_COLORS % threads_num);
    int per_thread = MAX_COLORS / threads_num;

    if(MAX_COLORS % threads_num == 0 || k < non_complementary){
        start_number = k * per_thread;
        end_number = (k + 1) * per_thread;
    }else{
        start_number = (non_complementary * per_thread) + (k - non_complementary) * (per_thread + 1);
        end_number = start_number + per_thread + 1;
    }

    int size = IMAGE_DATA.height * IMAGE_DATA.width;
    int val;
    ForRange(0, size){
        val = IMAGE_DATA.data[i / IMAGE_DATA.width][i % IMAGE_DATA.width];
        if(start_number <= val && val < end_number){
//            histogram[val]++; //ohhhh
            atomic_fetch_add(&histogram[val], 1);
        }
    }

    return get_time(&start);
}

int min(int a, int b){
    return (a < b) ? a : b;
}

long block_mode_thread(int *tid){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int k = *tid;
    int cap = min((k + 1) * ceil((double) IMAGE_DATA.width / threads_num), IMAGE_DATA.width);

    int part_size = IMAGE_DATA.width / threads_num;
    ForRange(k * ceil((double) IMAGE_DATA.width / threads_num), cap){
        For(row, 0, IMAGE_DATA.height, 1){
            atomic_fetch_add(&histogram[IMAGE_DATA.data[row][i]], 1);
        }
    }

    return get_time(&start);
}

long interleaved_mode_thread(int *tid){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int k = *tid;
    For(column, k, IMAGE_DATA.width, threads_num){
        For(row, 0, IMAGE_DATA.height, 1){
            atomic_fetch_add(&histogram[IMAGE_DATA.data[row][column]], 1);
        }
    }

    return get_time(&start);
}

void save_histogram(const char *out){
    const int RES_HEIGHT = 300;
    const int RES_WIDTH_MULT = 5;
    const int RES_WIDTH = MAX_COLORS * RES_WIDTH_MULT;

    FILE *f = fopen(out, "w+");
    if(f == NULL){
        printf("Cannot open file [%s] for write", out);
        exit(EXIT_FAILURE);
    }

    int max = histogram[0];
    ForRange(1, MAX_COLORS){
        if(histogram[i] > max){
            max = histogram[i];
        }
    }

    fputs("P2\n", f);
    fprintf(f, "%d %d\n", RES_WIDTH, RES_HEIGHT);
    fprintf(f, "%d\n", MAX_COLORS-1);

    double max_d = max;
    For(row, 0, RES_HEIGHT, 1){
        ForRange(0, MAX_COLORS){
            if(RES_HEIGHT - row < RES_HEIGHT * (histogram[i] / max_d)){
                For(k, 0, RES_WIDTH_MULT, 1){
                    fprintf(f, "%d ", histogram[i]);
                }
            }else{
                For(k, 0, RES_WIDTH_MULT, 1){
                    fprintf(f, "0 ");
                }
            }
        }
        fputs("\n", f);
    }

    fclose(f);
}

int main(int argc, char *argv[]){
    if(argc != 5){
        puts("main threads_num sign/block/interleaved input_file output_file");
        return 1;
    }

    threads_num = atoi(argv[1]);
    int mode = parse_mode(argv[2]);
    char *input_file = argv[3];
    char *output_file = argv[4];

    load_image(input_file);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    long (*mode_function[3])(int *) = {
            sign_mode_thread,
            block_mode_thread,
            interleaved_mode_thread
    };

    pthread_t *threads = calloc(threads_num, sizeof(pthread_t));
    int *tids = calloc(threads_num, sizeof(int));
    ForRange(0, threads_num){
        tids[i] = i;
        pthread_create(threads + i, NULL, (void *(*)(void *)) mode_function[mode], (void *)(tids + i));
    }

    long ret;
    ForRange(0, threads_num){
        pthread_join(threads[i], (void *) &ret);
        printf("Thread %lu took %ld microseconds\n", threads[i], ret);
    }

    printf("Main thread took %ld microseconds\n", get_time(&start));

    save_histogram(output_file);

    ForRange(0, IMAGE_DATA.height){
        free(IMAGE_DATA.data[i]);
    }
    free(IMAGE_DATA.data);
    free(threads);
    free(tids);

    return 0;
}