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
#include <stdbool.h>



#define MAX_CONNECTIONS 64
#define MAX_CLIENTS 32
#define MAX_MESSAGE_LEN 128

typedef enum{
    REGISTER_CLIENT = 0,
    NAME_IN_USE,
    NO_FREE_SLOTS,
    WAITING_FOR_OPPONENT,
    DISCONNECT,
    START_GAME,
    SERVER_SHUTDOWN,
    PING,
    TIMEOUT,
    OPP_DISCONNECTED,
    MOVE
} kCommand;

typedef enum{
    NOT_CONNECTED = 0,
    CONNECTED
} kStatus;

enum{
    NO_OPP = -1
} kOppStatus;

typedef enum{
    PING_GOT = 0,
    PING_SENT
} kPingStatus;

typedef enum {
    YOUR_MOVE = 0,
    OPP_MOVE,
    GAME_FINISHED
} kGameStatus;

#define BOARD_SIZE 3 //dont change pls
typedef enum{
    EMPTY = 0,
    X,
    O
} kSymbol;
const char symbols_str[3] = {' ', 'X', 'O'};

void close_socket(int fd){
    shutdown(fd, SHUT_RDWR);
    close(fd);
}