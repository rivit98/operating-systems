#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

#define ForRange(START_VALUE, END_VALUE) for(int i = START_VALUE; i < END_VALUE; i += 1)

const int RETRY_TIME = 5;
const int SPAWN_DELAY = 2;
const int SHAVING_TIME = 5;

struct {
    int *data;
    int max_size;
    int size;
} queue;

pthread_mutex_t queue_mutex        = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t somebody_in_queue   = PTHREAD_COND_INITIALIZER;
bool barber_working = false;
int clients_num;

int get_random_time(int k){
    return 1 + (rand() % k);
}

int get_free_place(void){
    int free_index = -1;
    ForRange(0, queue.max_size){
        if(queue.data[i] == 0){
            free_index = i;
            break;
        }
    }
    return free_index;
}

int get_waiting_client(void){
    ForRange(0, queue.max_size){
        if(queue.data[i] != 0){
            return i;
        }
    }

    return -1;
}

void *barber_worker() {
    int served_customers = 0;
    while(served_customers < clients_num){
        pthread_mutex_lock(&queue_mutex);

        while(queue.size == 0){
            barber_working = false;
            printf("Barber: going to sleep\n");
            pthread_cond_wait(&somebody_in_queue, &queue_mutex);
        }
        barber_working = true;

        int cid = get_waiting_client();
        if(cid != -1){
            queue.size--;
            served_customers++;
            printf(GRN "Barber: %d clients waiting, shaving client: %d\n" RESET, queue.size, queue.data[cid]);
            queue.data[cid] = 0;
        }
        pthread_mutex_unlock(&queue_mutex);
        sleep(get_random_time(SHAVING_TIME));
    }
    return NULL;
}

void client_worker(int *tid){
    int id = *tid;
    pthread_mutex_lock(&queue_mutex);

    int free_index = get_free_place();
    if(free_index == -1){
        pthread_mutex_unlock(&queue_mutex);
        printf(YEL "Client: Busy; %d\n" RESET, id);
        sleep(get_random_time(RETRY_TIME));
        client_worker(tid);
        return;
    }

    queue.data[free_index] = id;
    queue.size++;

    if(queue.size == 1 && !barber_working){
        printf("Client: Waking up barber; %d\n", id);
        pthread_cond_signal(&somebody_in_queue);
//        pthread_cond_broadcast(&somebody_in_queue);
    }else{
        printf(BLU "Client: Waiting room, free seats: %d; %d\n" RESET, queue.max_size - queue.size, id);
    }

    pthread_mutex_unlock(&queue_mutex);
}

int main(int argc, char **argv){
    if(argc != 3){
        puts("main chairs_num clients_num");
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));

    int K = atoi(argv[1]); //chairs
    clients_num = atoi(argv[2]); //clients

    queue.size = 0;
    queue.max_size = K;
    queue.data = calloc(K, sizeof(int));

    pthread_t barber;
    pthread_create(&barber, NULL, barber_worker, NULL);

    pthread_t *threads = calloc(clients_num, sizeof(pthread_t));
    int *tids = calloc(clients_num, sizeof(int));
    ForRange(0, clients_num){
        tids[i] = i+1;
        sleep(get_random_time(SPAWN_DELAY));
        pthread_create(threads + i, NULL, (void *(*)(void *)) client_worker, (void *)(tids + i));
    }

    pthread_join(barber, NULL);
    ForRange(0, clients_num){
        pthread_join(threads[i], NULL);
    }

    free(tids);
    free(threads);
    free(queue.data);

    return 0;
}