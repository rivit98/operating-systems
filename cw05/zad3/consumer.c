#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
    if(argc != 4){
        printf("Usage: main fifo filename chars_per_read\n");
        return 0;
    }

    size_t chars_per_read = atoi(argv[3]);

    FILE *f_pipe = fopen(argv[1], "r");
    if(!f_pipe){
        printf("error during opening fifo");
        return 1;
    }

    FILE *f_out = fopen(argv[2], "w");
    if(!f_out){
        printf("error during opening file out");
        return 1;
    }

    char buffer[256];
    size_t len = 0;
    while((len = fread(&buffer[0], sizeof(char), chars_per_read, f_pipe)) > 0){
        fwrite(buffer, sizeof(char), len, f_out);
    }

    fflush(f_out);
    fclose(f_pipe);
    fclose(f_out);

    return 0;
}