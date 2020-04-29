#include "common.h"

sem_t *semaphores[4];
orders_mem *smem;
bool running = true;

void open_resources(){
    semaphores[AVAILABLE_TASKS] = sem_open(SEM_TASKS, O_RDWR);
    semaphores[WORKER2] = sem_open(SEM_WORKER2, O_RDWR);
    semaphores[WORKER3] = SEM_FAILED;
    semaphores[MEMORY_LOCK] = sem_open(SEM_MEM, O_RDWR);
    int shared_memory_id = shm_open(SHARED_MEM_NAME, O_RDWR, PERMISSIONS);
    smem = mmap(NULL, sizeof(orders_mem), PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
}

void close_sema(){
    running = false;
}

int main(){
    srand(time(NULL) + getpid());
    signal(SIGINT, close_sema);
    open_resources();

    long ms;
    time_t s;
    while(running){
        sem_wait(semaphores[AVAILABLE_TASKS]);
        sem_wait(semaphores[MEMORY_LOCK]);

        int o_index = (smem->start_index + smem->orders_num) % MAX_TASKS;
        if(smem->orders[o_index].status == NONE) {
            smem->orders[o_index] = (Order) {.val = rand() % 10000, .status = ADDED};
            smem->to_prepare++;
            smem->orders_num++;

            get_current_time_with_ms(&s, &ms);
            printf(GRN "(%d %ds.%03ldms) Dodalem liczbe: %d. "
                   "Liczba zamowien do przygotowania: %d. "
                   "Liczba zamowien do wyslania: %d\n" RESET,
                   getpid(), (int) s, ms,
                   smem->orders[o_index].val, smem->to_prepare, smem->to_sent);
        }

        sem_post(semaphores[MEMORY_LOCK]);
        sem_post(semaphores[WORKER2]);

        usleep(700000 + getpid() + rand() % 100000);
    }

    for(int i = 0; i < 4; i ++){
        if(semaphores[i] != SEM_FAILED){
            sem_close(semaphores[i]);
        }
    }
    munmap(smem, sizeof(orders_mem));

    return 0;
}