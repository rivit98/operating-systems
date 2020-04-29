#pragma once

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>


#define SEM_MEM "/memory"
#define SEM_TASKS "/tasks"
#define SEM_WORKER2 "/worker2"
#define SEM_WORKER3 "/worker3"
#define SHARED_MEM_NAME "/shm_mem"
#define PERMISSIONS 0666
#define MAX_TASKS 10

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