#pragma once

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>


#define SEMAPHORE_PROJECT_ID 1234
#define MEMORY_PROJECT_ID 1235
#define MAX_TASKS 10
#define PERMISSION 0666

enum {
    AVAILABLE_TASKS = 0,
    WORKER2,
    WORKER3,
    MEMORY_LOCK
};

typedef enum{
    NONE = 0,
    ADDED,
    PACKED
} OrderStatus;

typedef struct{
    int val;
    OrderStatus status;
} Order;

typedef struct{
    int to_sent;
    int to_prepare;
    int orders_num;
    int start_index;
    Order orders[MAX_TASKS];
} orders_mem;

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO */
};

struct sembuf decrease_available_products = { AVAILABLE_TASKS, -1, 0};
struct sembuf increase_available_products = { AVAILABLE_TASKS, 1, 0};

struct sembuf increase_worker2 = {WORKER2, 1, 0};
struct sembuf decrease_worker2 = {WORKER2, -1, 0};

struct sembuf increase_worker3 = {WORKER3, 1, 0};
struct sembuf decrease_worker3 = {WORKER3, -1, 0};

struct sembuf mem_lock = {MEMORY_LOCK, -1, 0 };
struct sembuf mem_unlock = { MEMORY_LOCK, 1, 0 };


void get_current_time_with_ms(time_t *s, long *ms)
{
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    *s  = spec.tv_sec;
    *ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (*ms > 999) {
        (*s)++;
        *ms = 0;
    }
}

#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"