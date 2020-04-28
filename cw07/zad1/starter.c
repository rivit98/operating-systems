#include <wait.h>
#include <string.h>
#include <errno.h>

#include "common.h"

int semaphore_id;
int shared_memory_id;
pid_t *childs;
size_t total_workers;

void create_semaphore(){
    key_t sem_key = ftok(getenv("HOME"), SEMAPHORE_PROJECT_ID);
    semaphore_id = semget(sem_key, 4, IPC_CREAT | PERMISSION);

    semctl(semaphore_id, 0, SETVAL, (union semun) {.val = MAX_TASKS}); //available, task space
    semctl(semaphore_id, 1, SETVAL, (union semun) {.val = 0}); //worker2
    semctl(semaphore_id, 2, SETVAL, (union semun) {.val = 0}); //3
    semctl(semaphore_id, 3, SETVAL, (union semun) {.val = 1}); //mem lock
}

void create_shared_memory(){
    key_t sem_key = ftok(getenv("HOME"), MEMORY_PROJECT_ID);
    shared_memory_id = shmget(sem_key, sizeof(orders_mem), IPC_CREAT | PERMISSION);

    orders_mem *mem = shmat(shared_memory_id, NULL, 0);
    memset(mem, 0, sizeof(orders_mem));
    shmdt((const void *) mem);
}

void cleanup(){
    if(shared_memory_id != -1){
        shmctl(shared_memory_id, IPC_RMID, 0);
    }

    if(semaphore_id != -1){
        semctl(semaphore_id, 0, IPC_RMID);
    }

    free(childs);
    shared_memory_id = -1;
    semaphore_id = -1;
    childs = NULL;
}

void kill_childs(){
    for(int i = 0; i < total_workers; i++){
        kill(childs[i], SIGINT);
    }
    cleanup();
    exit(0);
}

int main(int argc, char *argv[]){
    if(argc != 4){
        puts("starter num1 num2 num3");
        return 0;
    }

    signal(SIGINT, kill_childs);
    signal(SIGTERM, kill_childs);

    int w1 = atoi(argv[1]);
    int w2 = atoi(argv[2]);
    int w3 = atoi(argv[3]);
    total_workers = w1 + w2 + w3;
    childs = calloc(total_workers, sizeof(pid_t));
    int child_indexer = 0;

    create_semaphore();
    create_shared_memory();

    for(int i = 0; i < w1; i++){
        pid_t child = fork();
        if(child == 0){
            execl("./worker1", "./worker1", NULL);
            exit(0);
        }
        childs[child_indexer++] = child;
    }
    for(int i = 0; i < w2; i++){
        pid_t child = fork();
        if(child == 0){
            execl("./worker2", "./worker2", NULL);
            exit(0);
        }
        childs[child_indexer++] = child;
    }
    for(int i = 0; i < w3; i++){
        pid_t child = fork();
        if(child == 0){
            execl("./worker3", "./worker3", NULL);
            exit(0);
        }
        childs[child_indexer++] = child;
    }

    for(int i = 0; i < total_workers; i++){
        wait(0);
    }

    cleanup();
    return 0;
}