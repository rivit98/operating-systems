#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <mqueue.h>

#define MAX_MESSAGE_LEN  0x800
#define MESSAGE_SIZE (MAX_MESSAGE_LEN - 1 - (sizeof(((Message *)0)->type)))
#define MAX_CLIENTS 0x10

const uint PERMISSIONS = 0666;

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
    char type;
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
    mqd_t que_id;
    int index;
    kClientStatus status;
    char fname[128];
} ClientData;

void dump_message(Message *m){
    printf("---------------\n");
    printf("> Type: %s\n", kTaskTypeStr[(int)m->type]);
    printf("> Index: %d\n", m->index);
    printf("> Text: %s\n", m->text);
    printf("---------------\n");
}

bool starts_with(const char *s, const char *prefix){
    return strncmp(prefix, s, strlen(prefix)) == 0;
}

int get_message(mqd_t source, Message *m) {
    char message[MAX_MESSAGE_LEN];
    char num[5];
    int ret = mq_receive(source, &message[0], MAX_MESSAGE_LEN, NULL);
    if (ret == -1){
        return 0;
    }
    m->type = message[0];
    for(int i = 0; i < 4; i++){
        num[i] = message[i+1];
    }
    num[4] = '\0';
    sscanf(&num[0], "%d", &m->index);
    sprintf(&m->text[0], "%s", message + 5);
    return ret;
}

int send_message2(mqd_t destination, Message *m) {
    uint length = strlen(&m->text[0]) + sizeof(m->type) + sizeof(m->index) + 1;
    char buffer[length];
    buffer[0] = m->type;
    sprintf(buffer + 1, "%04d", m->index);
    sprintf(buffer + 5, "%s", &m->text[0]);
    int ret = mq_send(destination, &buffer[0], length, 0);
    if(ret == -1){
        perror("send_message");
        exit(EXIT_FAILURE);
    }

    return ret;
}

int send_message(mqd_t destination, char type, int index, char *msg) {
    Message m;
    m.type = type;
    m.index = index;
    if(msg == NULL){
        m.text[0] = '\0';
    }else{
        strncpy(&m.text[0], msg, MESSAGE_SIZE);
    }
    return send_message2(destination, &m);
}