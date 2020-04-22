#include "common.h"

//ye globals, but I don't like passing this all the time...
static int client_queue_id;
static int server_queue_id;
static int client_index;
static int connected_with_client = -1;

void cleanup();

int initialize_connection(key_t key){
    server_queue_id = create_msg_queue(SERVER_KEY_FILEPATH, PROJECT_ID, 0);
    Message message = { .type = INIT,
                        .index = key};

    sprintf(&message.text[0], "%d", getpid());
    if(msgsnd(server_queue_id, &message, MESSAGE_SIZE, 0) == -1){
        perror("msgsnd initialize_connection");
        exit(EXIT_FAILURE);
    }

    if(msgrcv(client_queue_id, &message, MESSAGE_SIZE, INIT, 0) == -1){
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    int ret = atoi(message.text);
    if(ret == -1){
        printf("No free slots\n");
        _Exit(0);
    }

    return message.index;
}

void list_handler(Message *m){
    m->type = LIST;
    if(msgsnd(server_queue_id, m, MESSAGE_SIZE, 0) == -1){
        perror("msgsnd parse_and_send LIST");
        exit(EXIT_FAILURE);
    }
    if(msgrcv(client_queue_id, m, MESSAGE_SIZE, LIST, 0) == -1){
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    printf("%s", m->text);
    fflush(stdout);
}

void switch_mode(Message *m){
    connected_with_client = atoi(m->text);
    if(connected_with_client != -1){
        printf("Switched mode to chat with %d\n", connected_with_client);
        fcntl(0, F_SETFL, O_NONBLOCK);
    }else{
        printf("Client not found or busy\n");
        connected_with_client = -1;
    }
}

void connect_handler(Message *m, int cid){
    m->type = CONNECT;
    sprintf(&m->text[0], "%d", cid);
    if(msgsnd(server_queue_id, m, MESSAGE_SIZE, 0) == -1){
        perror("msgsnd parse_and_send LIST");
        exit(EXIT_FAILURE);
    }

    if(msgrcv(client_queue_id, m, MESSAGE_SIZE, CONNECT, 0) == -1){
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    switch_mode(m);
}

void disconnect_handler(Message *m){
    m->index = client_index;
    m->type = DISCONNECT;
    snprintf(&m->text[0], MESSAGE_SIZE, "%d", connected_with_client);
    if(msgsnd(server_queue_id, m, MESSAGE_SIZE, 0) == -1){
        perror("msgsnd disconnect_handle");
        exit(EXIT_FAILURE);
    }
    printf("You left the chat.\n");
    connected_with_client = -1;
}

void stop_handler(){
    Message m;

    m.type = STOP;
    m.index = client_index;
    snprintf(&m.text[0], MESSAGE_SIZE, "%d", connected_with_client);
    if(msgsnd(server_queue_id, &m, MESSAGE_SIZE, 0) == -1){
        perror("msgsnd stop_handler");
        exit(EXIT_FAILURE);
    }
}

void disconnect(){
    printf("Your friend left the chat.\n");
    connected_with_client = -1;
}

void parse_and_send(Message *m, char *buf){
    if(starts_with(buf, "STOP") || starts_with(buf, "stop")){ //this really should be equal, but...
        cleanup();
    }

    if(connected_with_client == -1){
        if(starts_with(buf, "LIST") || starts_with(buf, "list")){
            list_handler(m);
        }else if(starts_with(buf, "CONNECT") || starts_with(buf, "connect")){
            int cid = atoi(&buf[8]);
            connect_handler(m, cid);
        }else{
            printf("Wrong command!\n");
        }
    }else{
        if(starts_with(buf, "DISCONNECT") || starts_with(buf, "disconnect")){
            disconnect_handler(m);
        }else{
            strncpy(&m->text[0], &buf[0], MESSAGE_SIZE);
            if(msgsnd(connected_with_client, m, MESSAGE_SIZE, 0) == -1){
                perror("msgsnd sending to client");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void listen(key_t key){
    connected_with_client = -1;
    client_index = initialize_connection(key);
    printf("My ID: %d\n", client_index);

    Message message;
    char buf[512];
    bool no_message = true;
    char *ptr;

    for(;;){
        if(connected_with_client == -1){
            //fgets is blocking
            if(fgets(&buf[0], sizeof(buf), stdin) != NULL){
                message.index = client_index;
                message.text[0] = '\0';
                parse_and_send(&message, &buf[0]);
                memset(&message, 0, sizeof(Message));
            }
        }else{
            no_message = false;
            if(msgrcv(client_queue_id, &message, MESSAGE_SIZE, TEXT, IPC_NOWAIT) == -1){
                no_message = true;
                if(errno != ENOMSG){
                    perror("msgrcv reading from client");
                    exit(EXIT_FAILURE);
                }
            }

            if(!no_message){
                printf("%50s", message.text);
                fflush(stdout);
            }

            if(read(0, &buf[0], sizeof(buf)) < 0){
                //nothing to read
            }else{
                ptr = strchr(&buf[0], '\n');
                ptr++;
                *ptr = '\0';
                if(buf[0] != '\0' && buf[1] != '\0'){
                    message.index = client_queue_id;
                    message.type = TEXT;
                    parse_and_send(&message, &buf[0]);
                }

                memset(&message, 0, sizeof(Message));
                memset(&buf[0], 0, sizeof(buf));
            }

            usleep(200000);
        }
    }
}

void cleanup(){
    stop_handler();
    msgctl(client_queue_id, IPC_RMID, NULL);
    _Exit(0);
}

void process_message(){
    Message m;
    if(msgrcv(client_queue_id, &m, MESSAGE_SIZE, 0, 0) == -1){
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    switch (m.type) {
        case STOP:{
            cleanup();
            break;
        }
        case CONNECT:{
            switch_mode(&m);
            break;
        }
        case DISCONNECT:{
            disconnect();
            break;
        }
    }
}

int main(int argc, char *argv[]){
    atexit(cleanup);
    signal(SIGINT, cleanup);
    signal(WAKEUP_SIGNAL, process_message);

    const char *home_dir = getenv("HOME");
    if(home_dir == NULL){
        home_dir = argv[0];
    }

    client_queue_id = create_msg_queue(home_dir, getpid(), IPC_CREAT);

    //quick fix :(
    key_t key;
    if((key = ftok(home_dir, getpid())) == -1){
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    //
    listen(key);

    return 1;
}