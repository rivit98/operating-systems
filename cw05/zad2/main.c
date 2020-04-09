#include <stdio.h>

int main(int argc, char **argv){
    if(argc != 2){
        printf("Usage: main filename\n");
        return 0;
    }

    char buffer[128];
    sprintf(&buffer[0], "cat %s | sort", argv[1]);

    FILE *f = popen(&buffer[0], "r");
    if(!f){
        printf("popen error\n");
        return 1;
    }

    while(fgets(&buffer[0], sizeof(buffer), f) != NULL){
        printf("%s", buffer);
    }

    pclose(f);
    return 0;
}