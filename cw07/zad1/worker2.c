#include "common.h"


int main(){
    key_t sem_key = ftok(getenv("HOME"), SEMAPHORE_PROJECT_ID);
    int semaphore_id = semget(sem_key, 0, 0666);

    sem_key = ftok(getenv("HOME"), MEMORY_PROJECT_ID);
    int shared_memory_id = shmget(sem_key, 0, PERMISSION);

    long ms;
    time_t s;
    while(true){
        semop(semaphore_id, (struct sembuf[2]){decrease_worker2, mem_lock}, 2);
        orders_mem *smem = shmat(shared_memory_id, NULL, 0);

        for(int i = smem->start_index; i < smem->start_index + smem->orders_num; i++){
            int o_index = i % MAX_TASKS;
            if(smem->orders[o_index].status != ADDED){
                continue;
            }

            smem->orders[o_index].val *= 2;
            smem->orders[o_index].status = PACKED;
            smem->to_prepare--;
            smem->to_sent++;

            get_current_time_with_ms(&s, &ms);
            printf(YEL "(%d %ds.%03ldms) Przygotowałem zamówienie o wielkości %d. "
                   "Liczba zamównień do przygotowania: %d. "
                   "Liczba zamównień do wysłania: %d.\n" RESET,
                   getpid(), (int)s, ms,
                   smem->orders[o_index].val, smem->to_prepare, smem->to_sent);
            break; //po jednym
        }

        shmdt(smem);
        semop(semaphore_id, (struct sembuf[2]){increase_worker3, mem_unlock}, 2);
//        sleep(1 + getpid() % 3);
    }
}