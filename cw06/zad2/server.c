#include "common.h"

mqd_t server_queue_id;
ClientData clients[MAX_CLIENTS];

int get_free_index(){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].status == NOT_CONNECTED){
            return i;
        }
    }

    return -1;
}

void handle_message(Message *message) { //this should be splitted into separate functions
    int cid = message->index;
    switch (message->type) {
        case INIT: {
            int msgid = mq_open(message->text, O_RDWR, PERMISSIONS, NULL);
            Message m = {.type = INIT};
            int free_client_index = get_free_index();
            if (free_client_index == -1) { //no free places
                sprintf(&m.text[0], "%d", -1);
                if (send_message2(msgid, &m) == -1) {
                    perror("msgsnd CONNECT bad");
                    return;
                }
                return;
            }
            clients[free_client_index].index = free_client_index;
            clients[free_client_index].status = CONNECTED;
            clients[free_client_index].que_id = msgid;
            strncpy(&clients[free_client_index].fname[0], &message->text[0], 127);
            clients[free_client_index].fname[strlen(&clients[free_client_index].fname[0])] = '\0';

            if (send_message(clients[free_client_index].que_id, INIT, clients[free_client_index].index, NULL) == -1) {
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
            if (send_message2(clients[cid].que_id, &m) == -1) {
                perror("msgsnd LIST");
                return;
            }

            break;
        }
        case CONNECT:{
            Message m = {.type = CONNECT};
            m.index = -1;
            int to_connect_cid = atoi(message->text);
            if(to_connect_cid < 0 || to_connect_cid >= MAX_CLIENTS || clients[to_connect_cid].status == NOT_CONNECTED){
                printf("> CONNECT | cid: %d, to: %d | CLIENT NOT FOUND\n", clients[cid].index, to_connect_cid);
                m.text[0] = '\0';
                if (send_message2(clients[cid].que_id, &m) == -1) {
                    perror("msgsnd CONNECT bad");
                    return;
                }
                return;
            }

            if(clients[to_connect_cid].status == BUSY || clients[cid].status == BUSY){
                printf("> CONNECT | cid: %d, to: %d | CLIENT BUSY\n", clients[cid].index, to_connect_cid);
                m.text[0] = '\0';
                if (send_message2(clients[cid].que_id, &m) == -1) {
                    perror("msgsnd CONNECT bad");
                    return;
                }
                return;
            }

            m.index = to_connect_cid;
            sprintf(&m.text[0], "%s", clients[to_connect_cid].fname);
            if (send_message2(clients[cid].que_id, &m) == -1) {
                perror("msgsnd CONNECT to");
                return;
            }

            m.index = cid;
            sprintf(&m.text[0], "%s", clients[cid].fname);
            if(send_message2(clients[to_connect_cid].que_id, &m) == -1){
                perror("msgsnd CONNECT from");
                return;
            }

            clients[cid].status = BUSY;
            clients[to_connect_cid].status = BUSY;
            printf("> CONNECT | cid: %d, to: %d\n", clients[cid].index, to_connect_cid);
            break;
        }
        case DISCONNECT:{
            int to_disconnect_cid = atoi(message->text);
            Message m = {.type = DISCONNECT, .index = to_disconnect_cid, .text[0] = '\0'};
            if(to_disconnect_cid < 0 || to_disconnect_cid >= MAX_CLIENTS || clients[to_disconnect_cid].status == NOT_CONNECTED){
                printf("> DISCONNECT | cid: %d, to: %d | CLIENT NOT FOUND\n", clients[cid].index, to_disconnect_cid);
                return;
            }

            clients[cid].status = CONNECTED;
            if (send_message2(clients[to_disconnect_cid].que_id, &m) == -1) {
                perror("msgsnd DISCONNECT from");
                return;
            }

            clients[to_disconnect_cid].status = CONNECTED;
            printf("> DISCONNECT | cid: %d, to: %d\n", clients[cid].index, to_disconnect_cid);
            break;
        }
        case STOP: {
            printf("> STOP | cid: %d\n", clients[cid].index);

            int to_disconnect_cid = atoi(message->text);
            if(to_disconnect_cid != -1){
                message->type = DISCONNECT;
                handle_message(message);
            }

            clients[cid].index = -1;
            clients[cid].status = NOT_CONNECTED;
            clients[cid].que_id = 0;
            clients[cid].fname[0] = '\0';

            break;
        }
        default: //just ignore or send something
            break;
    }
}

void listen(){
    Message m;
    memset(&clients[0], 0, MAX_CLIENTS * sizeof(ClientData));

    while(1){
        if(get_message(server_queue_id, &m) > 0){
            handle_message(&m);
        }
    }
}

void shudown_clients(){
    Message m;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if (clients[i].status == NOT_CONNECTED) {
            continue;
        }

        m.text[0] = '\0';
        m.type = STOP;
        m.index = i;
        if (send_message2(clients[i].que_id, &m) == -1) {
            perror("msgsnd shudown_clients");
            return;
        }
        memset(&m, 0, sizeof(Message));
        if (!get_message(server_queue_id, &m)) {
            perror("msgsnd shudown_clients");
            return;
        }
        if(m.type == STOP){
            mq_close(clients[i].que_id);
            printf("Shutting down client: %d\n", i);
        }
    }

    mq_close(server_queue_id);
    mq_unlink("/server");
    _Exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]){
    signal(SIGINT, shudown_clients);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_LEN;
    attr.mq_curmsgs = 0;
    server_queue_id = mq_open("/server", O_RDWR | O_CREAT, PERMISSIONS, &attr);
    if(server_queue_id == -1){
        perror("mq_open");
        exit(1);
    }
    printf("Created server queue %d\n", server_queue_id);
    listen();

    shudown_clients();

    return 1;
}



