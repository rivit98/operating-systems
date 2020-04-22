#include "common.h"

static int server_queue_id;
ClientData clients[MAX_CLIENTS];

int get_free_index(){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].status == NOT_CONNECTED){
            return i;
        }
    }

    return -1;
}

int get_client_by_que_id(int q){
    int to_disconnect_cid = -1;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].que_id == q){
            to_disconnect_cid = i;
        }
    }
    return to_disconnect_cid;
}

void handle_message(Message *message) { //this should be splitted into separate functions
//    printf("\nReceived: %s from cid %d | TYPE %s\n", message->text, message->index, kTaskTypeStr[message->type]);
    int cid = message->index;
    switch (message->type) {
        case INIT: {
            int msgid;
            if ((msgid = msgget((key_t) cid, PERMISSIONS)) == -1) {
                perror("msgget INIT");
                return;
            }
            Message m = {.type = INIT};
            int free_client_index = get_free_index();
            if (free_client_index == -1) { //no free places
                sprintf(&m.text[0], "%d", -1);
                if (msgsnd(msgid, &m, MESSAGE_SIZE, 0) == -1) {
                    perror("msgsnd CONNECT bad");
                    return;
                }
                return;
            }

            clients[free_client_index].index = free_client_index;
            clients[free_client_index].status = CONNECTED;
            clients[free_client_index].que_id = msgid;
            clients[free_client_index].pid = atoi(message->text);
            m.index = clients[free_client_index].index;

            if (msgsnd(clients[free_client_index].que_id,
                       &(Message) {.type = INIT, .index = clients[free_client_index].index, .text[0] = '\0'},
                       MESSAGE_SIZE, 0) == -1) {
                perror("msgsnd INIT");
                return;
            }

            printf("> INIT | cid: %d, que_id: %d\n",
                   clients[free_client_index].index,
                   clients[free_client_index].que_id);
            break;
        }
        case LIST: {
            printf("> LIST | cid: %d, que_id: %d\n", clients[cid].index, clients[cid].que_id);

            Message m = {.type = LIST, .index = cid};
            int len = 0;
            len += snprintf(&m.text[len], MAX_MESSAGE_LEN - len, "You    %d -> %s\n", clients[cid].index,
                            kClientStatusStr[clients[cid].status]);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].status == NOT_CONNECTED || cid == i) {
                    continue;
                }
                len += snprintf(&m.text[len], MAX_MESSAGE_LEN - len, "Client %d -> %s\n", clients[i].index,
                            kClientStatusStr[clients[i].status]);
            }
            if (msgsnd(clients[cid].que_id, &m, MESSAGE_SIZE, 0) == -1) {
                perror("msgsnd LIST");
                return;
            }

            break;
        }
        case CONNECT:{
            Message m = {.type = CONNECT};
            int to_connect_cid = atoi(message->text);
            if(to_connect_cid < 0 || to_connect_cid >= MAX_CLIENTS || clients[to_connect_cid].status == NOT_CONNECTED){
                printf("> CONNECT | cid: %d, to: %d | CLIENT NOT FOUND\n", clients[cid].index, to_connect_cid);
                sprintf(&m.text[0], "%d", -1);
                if (msgsnd(clients[cid].que_id, &m, MESSAGE_SIZE, 0) == -1) {
                    perror("msgsnd CONNECT bad");
                    return;
                }
                return;
            }

            if(clients[to_connect_cid].status == BUSY || clients[cid].status == BUSY){
                printf("> CONNECT | cid: %d, to: %d | CLIENT BUSY\n", clients[cid].index, to_connect_cid);
                sprintf(&m.text[0], "%d", -1);
                if (msgsnd(clients[cid].que_id, &m, MESSAGE_SIZE, 0) == -1) {
                    perror("msgsnd CONNECT bad");
                    return;
                }
                return;
            }

            m.index = to_connect_cid;
            sprintf(&m.text[0], "%d", clients[to_connect_cid].que_id);
            if (msgsnd(clients[cid].que_id, &m, MESSAGE_SIZE, 0) == -1) {
                perror("msgsnd CONNECT to");
                return;
            }

            m.index = cid;
            sprintf(&m.text[0], "%d", clients[cid].que_id);
            if (msgsnd(clients[to_connect_cid].que_id, &m, MESSAGE_SIZE, 0) == -1) {
                perror("msgsnd CONNECT from");
                return;
            }
            kill(clients[to_connect_cid].pid, WAKEUP_SIGNAL);

            clients[cid].status = BUSY;
            clients[to_connect_cid].status = BUSY;
            printf("> CONNECT | cid: %d, to: %d\n", clients[cid].index, to_connect_cid);
            break;
        }
        case DISCONNECT:{
            int q = atoi(message->text);
            int to_disconnect_cid = get_client_by_que_id(q);
            Message m = {.type = DISCONNECT, .index = to_disconnect_cid, .text[0] = '\0'};
            if(to_disconnect_cid < 0 || to_disconnect_cid >= MAX_CLIENTS || clients[to_disconnect_cid].status == NOT_CONNECTED){
                printf("> DISCONNECT | cid: %d, to: %d | CLIENT NOT FOUND\n", clients[cid].index, to_disconnect_cid);
                return;
            }

            clients[cid].status = CONNECTED;
            if (msgsnd(clients[to_disconnect_cid].que_id, &m, MESSAGE_SIZE, 0) == -1) {
                perror("msgsnd DISCONNECT from");
                return;
            }
            kill(clients[to_disconnect_cid].pid, WAKEUP_SIGNAL);

            clients[to_disconnect_cid].status = CONNECTED;
            printf("> DISCONNECT | cid: %d, to: %d\n", clients[cid].index, to_disconnect_cid);
            break;
        }
        case STOP: {
            printf("> STOP | cid: %d\n", clients[cid].index);

            int q = atoi(message->text);
            int to_disconnect_cid = get_client_by_que_id(q);
            if(to_disconnect_cid != -1){
                message->type = DISCONNECT;
                handle_message(message);
            }

            clients[cid].index = -1;
            clients[cid].status = NOT_CONNECTED;
            clients[cid].que_id = 0;
            clients[cid].pid = 0;

            break;
        }
        default: //just ignore or send something
            break;
    }
}

void listen(){
    Message message;
    memset(&clients[0], 0, MAX_CLIENTS * sizeof(ClientData));

    for(;;){
        // priority handling
        if(
            msgrcv(server_queue_id, &message, MESSAGE_SIZE, STOP, IPC_NOWAIT) == -1 &&
            msgrcv(server_queue_id, &message, MESSAGE_SIZE, DISCONNECT, IPC_NOWAIT) == -1 &&
            msgrcv(server_queue_id, &message, MESSAGE_SIZE, LIST, IPC_NOWAIT) == -1 &&
            msgrcv(server_queue_id, &message, MESSAGE_SIZE, 0, IPC_NOWAIT) == -1){

            if(errno != ENOMSG){
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }else{
                usleep(300000);
                continue;
            }
        }

        handle_message(&message);
    }
}

void shudown_clients(){
    Message m;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if (clients[i].status == NOT_CONNECTED) {
            continue;
        }
        kill(clients[i].pid, 0); //check if pid exists
        if(errno == ESRCH){
            continue; //not exists
        }

        m.text[0] = '\0';
        m.type = STOP;
        m.index = i;
        if (msgsnd(clients[i].que_id, &m, MESSAGE_SIZE, 0) == -1) {
            perror("msgsnd shudown_clients");
            return;
        }
        kill(clients[i].pid, WAKEUP_SIGNAL);

        memset(&m, 0, sizeof(Message));
        if (msgrcv(server_queue_id, &m, MESSAGE_SIZE, STOP, 0) == -1) {
            perror("msgsnd shudown_clients");
            return;
        }
        if(m.type == STOP){
            printf("Shutting down client: %d\n", i);
        }
    }

    msgctl(server_queue_id, IPC_RMID, NULL); //notify clients before
    _Exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    signal(SIGINT, shudown_clients);

    server_queue_id = create_msg_queue(SERVER_KEY_FILEPATH, PROJECT_ID, IPC_CREAT);
    listen();

    return 1;
}