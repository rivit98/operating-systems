#pragma once

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>


#define MAX_CONNECTIONS 64
#define MAX_CLIENTS 32
#define MAX_MESSAGE_LEN 128

typedef enum{
    REGISTER_CLIENT = 0,
    NAME_IN_USE,
    NO_FREE_SLOTS,
    DISCONNECT,
    START_GAME,
    SERVER_SHUTDOWN,
    PING,
    TIMEOUT,
    OPP_DISCONNECTED

} kCommand;

typedef enum{
    NOT_CONNECTED = 0,
    CONNECTED
} kStatus;

enum{
    NO_OPP = -1
} kOppStatus;

typedef enum{
    NO_PING = 0,
    PING_SENT,
    PING_GOT
} kPingStatus;

void close_socket(int fd){
    shutdown(fd, SHUT_RDWR);
    close(fd);
}