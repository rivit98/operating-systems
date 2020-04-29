#include "common.h"

sem_t *semaphores[4];
orders_mem *smem;
bool running = true;

void open_resources(){
    semaphores[AVAILABLE_TASKS] = sem_open(SEM_TASKS, O_RDWR);
    semaphores[WORKER2] = SEM_FAILED;
    semaphores[WORKER3] = sem_open(SEM_WORKER3, O_RDWR);
    semaphores[MEMORY_LOCK] = sem_open(SEM_MEM, O_RDWR);
    int shared_memory_id = shm_open(SHARED_MEM_NAME, O_RDWR, PERMISSIONS);
    smem = mmap(NULL, sizeof(orders_mem), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_id, 0);
}

void close_sema(){
    running = false;
}

int main(){
    signal(SIGINT, close_sema);
    open_resources();

    long ms;
    time_t s;
    while(running){
        sem_wait(semaphores[WORKER3]);
        sem_wait(semaphores[MEMORY_LOCK]);

        for(int i = smem->start_index; i < smem->start_index + smem->orders_num; i++){
            int o_index = i % MAX_TASKS;
            if(smem->orders[o_index].status != PACKED){
                continue;
            }

            smem->orders[o_index].val *= 3;
            smem->orders[o_index].status = NONE;
            smem->to_sent--;
            smem->orders_num--;
            smem->start_index++;

            get_current_time_with_ms(&s, &ms);
            printf(BLU "(%d %ds.%03ldms) Wysłałem zamówienie o wielkości %d. "
                   "Liczba zamównień do przygotowania: %d. "
                   "Liczba zamównień do wysłania: %d.\n" RESET,
                   getpid(), (int)s, ms,
                   smem->orders[o_index].val, smem->to_prepare, smem->to_sent);
            break; //po jednym
        }

        sem_post(semaphores[AVAILABLE_TASKS]);
        sem_post(semaphores[MEMORY_LOCK]);
        sleep(1 + getpid() % 2);
    }

    for(int i = 0; i < 4; i ++){
        if(semaphores[i] != SEM_FAILED){
            sem_close(semaphores[i]);
        }
    }
    munmap(smem, sizeof(orders_mem));

    return 0;
}