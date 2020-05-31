#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define PIPE "./squareFIFO"

int main() {
    usleep(100000);
    int val = 0;
    /***********************************
    * odczytaj z potoku nazwanego PIPE zapisana tam wartosc i przypisz ja do zmiennej val
    * posprzataj
    ************************************/

    int fd = open(PIPE, O_RDONLY);
    if(fd == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    char buffer[128];
    if(read(fd, buffer, sizeof(buffer)) == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }

    close(fd);
    unlink(PIPE);

    val = atoi(buffer);
    
    printf("%d square is: %d\n", val, val*val);
    return 0;
}
