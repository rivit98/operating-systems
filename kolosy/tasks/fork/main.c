#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    int i, pid;

    if (argc != 2) {
        printf("Not a suitable number of program arguments");
        exit(2);
    } else {
        for (i = 0; i < atoi(argv[1]); i++) {
            //*********************************************************
            //Uzupelnij petle w taki sposob aby stworzyc dokladnie argv[1] procesow potomnych, bedacych dziecmi
            //   tego samego procesu macierzystego.
            // Kazdy proces potomny powinien:
            // - "powiedziec ktorym jest dzieckiem",
            //-  jaki ma pid,
            //- kto jest jego rodzicem
            //******************************************************

            if (fork() == 0) {
                printf("Jestem %d dzieckiem, moj PID to: %d, moim rodzicem jest %d\n", i + 1, getpid(), getppid());
                exit(EXIT_SUCCESS);
            }
        }

        for (i = 0; i < atoi(argv[1]); i++) {
            wait(NULL);
        }

    }
    return 0;
}
