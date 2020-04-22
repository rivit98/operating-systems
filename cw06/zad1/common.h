#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_MESSAGE_LEN  0x800
#define MESSAGE_SIZE (MAX_MESSAGE_LEN - 1 - (sizeof(((Message *)0)->type)))
#define MAX_CLIENTS 0x10
#define WAKEUP_SIGNAL SIGRTMIN

const char *SERVER_KEY_FILEPATH = "./server.c";
const uint PROJECT_ID = 0xDEAD;
const uint PERMISSIONS = 0644;

typedef enum {
    INIT = 1,
    LIST,
    CONNECT,
    DISCONNECT,
    STOP,
    ERROR,
    TEXT,
    LAST_TYPE
} kTaskType;

const char *kTaskTypeStr[LAST_TYPE] = {
        "",
        "INIT",
        "LIST",
        "CONNECT",
        "DISCONNECT",
        "STOP",
        "ERROR",
        "TEXT"
};

typedef struct{
    long type;
    int index;
    char text[MAX_MESSAGE_LEN];
} Message;

typedef enum{
    NOT_CONNECTED = 0,
    CONNECTED,
    BUSY,
    LAST_STATUS
} kClientStatus;

const char *kClientStatusStr[LAST_STATUS] = {
        "NOT CONNECTED",
        "CONNECTED",
        "BUSY"
};

typedef struct{
    int que_id;
    int index;
    int pid;
    kClientStatus status;
} ClientData;


int create_msg_queue(const char *path, int ftok_flags, int ipc_flags) {
    key_t key;

    if((key = ftok(path, ftok_flags)) == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int msgid;
    if((msgid = msgget(key, PERMISSIONS | ipc_flags)) == -1){
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    if(ipc_flags & IPC_CREAT){
        printf("> Queue created: %d\n", msgid);
    }else{
        printf("> Queue opened: %d\n", msgid);
    }

    return msgid;
}

void dump_message(Message *m){
    printf("---------------\n");
    printf("> Type: %s\n", kTaskTypeStr[m->type]);
    printf("> Index: %d\n", m->index);
    printf("> Text: %s\n", m->text);
    printf("---------------\n");
}

bool starts_with(const char *s, const char *prefix){
    return strncmp(prefix, s, strlen(prefix)) == 0;
}