#include <wait.h>
#include <string.h>

#include "common.h"

sem_t *semaphores[4];
pid_t *childs;
size_t total_workers;
int shared_memory_id;

void create_semaphore(){
    semaphores[AVAILABLE_TASKS] = sem_open(SEM_TASKS, O_CREAT, PERMISSIONS, MAX_TASKS);
    semaphores[WORKER2] = sem_open(SEM_WORKER2, O_CREAT, PERMISSIONS, 0);
    semaphores[WORKER3] = sem_open(SEM_WORKER3, O_CREAT, PERMISSIONS, 0);
    semaphores[MEMORY_LOCK] = sem_open(SEM_MEM, O_CREAT, PERMISSIONS, 1);

    for(int i = 0; i < 4; i ++){
        sem_close(semaphores[i]);
    }
}

void create_shared_memory(){
    shared_memory_id = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, PERMISSIONS);
    ftruncate(shared_memory_id, sizeof(orders_mem));

    orders_mem *mem = mmap(NULL, sizeof(orders_mem), PROT_WRITE | PROT_READ, MAP_SHARED, shared_memory_id, 0);
    memset(mem, 0, sizeof(orders_mem));
    munmap(mem, sizeof(orders_mem));
}

void cleanup(){
    if(shared_memory_id != -1){
        shm_unlink(SHARED_MEM_NAME);
        shared_memory_id = -1;
    }

    sem_unlink(SEM_TASKS);
    sem_unlink(SEM_WORKER2);
    sem_unlink(SEM_WORKER3);
    sem_unlink(SEM_MEM);

    free(childs);
    for(int i = 0; i < 4; i ++){
        semaphores[i] = SEM_FAILED;
    }
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
    atexit(cleanup);

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

    return 0;
}