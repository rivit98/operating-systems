#include "common.h"



int main(){
    srand(time(NULL) + getpid());
    key_t sem_key = ftok(getenv("HOME"), SEMAPHORE_PROJECT_ID);
    int semaphore_id = semget(sem_key, 0, 0666);

    sem_key = ftok(getenv("HOME"), MEMORY_PROJECT_ID);
    int shared_memory_id = shmget(sem_key, 0, PERMISSION);

    long ms;
    time_t s;
    while(true){
        semop(semaphore_id, (struct sembuf[2]){decrease_available_products, mem_lock}, 2);
        orders_mem *smem = shmat(shared_memory_id, NULL, 0);
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

        shmdt(smem);
        semop(semaphore_id, (struct sembuf[2]){increase_worker2, mem_unlock}, 2);
//        usleep(600000 + getpid() + rand() % 100000);
    }
}