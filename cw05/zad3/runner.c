#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char **argv){
    const char* fifoname = "./fifo_pipe";
    if(mkfifo(fifoname, 0666) != 0){
        printf("mkfifo error %s", fifoname);
        return 1;
    }


    if(fork() == 0){
        execlp("./consumer", "./consumer", fifoname, "out.txt", "5", NULL);
    }

    char buf[64];
    for(int i = 0; i < 5; ++i){
        if(fork() == 0){
            sprintf(buf, "in%d.txt", i+1);
            execlp("./producer", "./producer", fifoname, buf, "5", NULL);
        }
    }

    for(int i = 0; i < 6; ++i) {
        wait(NULL);
    }
    puts("Finished");

    return 0;
}