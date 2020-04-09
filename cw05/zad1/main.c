#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_LINE_LEN 2048
#define MAX_ARGS_NUM 32
#define WRITE_PIPE 1
#define READ_PIPE 0

void exec_commands(char **cmds, size_t task_num){
    int fd[2], prev_fd[2];
    if(pipe(fd) == -1 || pipe(prev_fd) == -1){
        printf("pipe error\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < task_num; i++){
        int pid_worker = fork();
        if(pid_worker < 0){
            printf("fork error\n");
            exit(EXIT_FAILURE);
        }

        //bedziemy czytac z prev_fd i zapisywac do fd
        //po zapisie bedziemy aktualizowac prev_fd

        if(pid_worker == 0){
            if(i != 0){ // jak pierwszy to nie ma z czego czytac
                dup2(prev_fd[READ_PIPE], STDIN_FILENO);
                close(prev_fd[WRITE_PIPE]);
            }
            dup2(fd[WRITE_PIPE], STDOUT_FILENO);

            int arg_num = 0;
            char *args[MAX_ARGS_NUM];
            memset(args, '\0', MAX_ARGS_NUM * sizeof(char *));
            char *arg = strtok(cmds[i], " ");
            while(arg != NULL){
                args[arg_num++] = strdup(arg);
                arg = strtok(NULL, " ");
            }

            execvp(args[0], &args[0]);
            exit(EXIT_SUCCESS);
        }

        close(fd[WRITE_PIPE]);
        waitpid(pid_worker, NULL, 0);
        prev_fd[READ_PIPE] = fd[READ_PIPE];
        prev_fd[WRITE_PIPE] = fd[WRITE_PIPE];
    }

    char output_buffer[512];
    while(read(fd[READ_PIPE], &output_buffer[0], sizeof(output_buffer)) > 0){
        puts(output_buffer);
    }
}

char *rtrim(char *s){
    char *back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

void load_commands(char *cmd_line){
    printf("\n\n%s", cmd_line);
    int arg_num = 1;
    char *c = rtrim(cmd_line);
    while(*c != '\0'){
        if(*c == '|'){
            arg_num++;
        }
        c++;
    }

    char **cmds = calloc(arg_num, sizeof(char *));
    for(int i = 0; i < arg_num; ++i){
        cmds[i] = calloc(MAX_LINE_LEN, sizeof(char *));
    }

    char *ptr = strtok(cmd_line, "|");
    size_t indexer = 0;
    while(ptr != NULL){
        strncpy(cmds[indexer++], ptr, MAX_LINE_LEN);
        ptr = strtok(NULL, "|");
    }

    exec_commands(cmds, arg_num);

    for(int i = 0; i < arg_num; ++i){
        free(cmds[i]);
    }
    free(cmds);
}

int main(int argc, char **argv){
    if(argc != 2){
        printf("Usage: main filename\n");
        return 0;
    }

    const char *filename = argv[1];
    FILE* f = fopen(filename, "r");
    if(!f){
        printf("Cannot open file [%s]", filename);
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_LINE_LEN];
    while(fgets(&buffer[0], sizeof(buffer), f) != NULL){
        load_commands(&buffer[0]);
    }

    fclose(f);

    return 0;
}