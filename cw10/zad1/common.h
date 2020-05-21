#pragma once

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define MAX_CONNECTIONS 64
#define MAX_CLIENTS 32
#define MAX_MESSAGE_LEN 128


typedef enum{
    REGISTER_CLIENT = 0,
    NAME_IN_USE,
    NO_FREE_SLOTS

} kCommand;

typedef enum{
    NOT_CONNECTED = 0,
    CONNECTED
} kStatus;

enum{
    NO_OPP = -1
} kOppStatus;