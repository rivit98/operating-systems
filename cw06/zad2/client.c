#include "common.h"

static mqd_t client_queue_id;
static mqd_t server_queue_id;
char fname[128];
static int client_index;
static mqd_t connected_with_client = -1;
static int connected_with_index = -1;

void cleanup();
void register_event();

int initialize_connection(){
    server_queue_id = mq_open("/server", O_RDWR, PERMISSIONS, NULL);
    if(server_queue_id == -1){
        perror("initialize_connection");
        exit(0);
    }
    printf("Opened server queue %d\n", server_queue_id);

    send_message(server_queue_id, INIT, 0, &fname[0]);
    Message message;
    if(get_message(client_queue_id, &message) == -1){
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

    if(send_message2(server_queue_id, m) == -1){
        perror("msgsnd parse_and_send LIST");
        exit(EXIT_FAILURE);
    }
}

void switch_mode(Message *m){
    if(m->index != -1){
        connected_with_client = mq_open(m->text, O_RDWR, PERMISSIONS, NULL);
        connected_with_index = m->index;
        printf("Switched mode to chat with %d\n", connected_with_index);
        fcntl(0, F_SETFL, O_NONBLOCK);
    }else{
        printf("Client not found or busy\n");
        connected_with_client = -1;
        connected_with_index = -1;
    }
}

void connect_handler(Message *m, int cid){
    m->type = CONNECT;
    sprintf(&m->text[0], "%d", cid);
    if(send_message2(server_queue_id, m) == -1){
        perror("msgsnd parse_and_send LIST");
        exit(EXIT_FAILURE);
    }

    if(get_message(client_queue_id, m) == -1){
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    switch_mode(m);
}

void disconnect_handler(Message *m){
    m->index = client_index;
    m->type = DISCONNECT;
    snprintf(&m->text[0], MESSAGE_SIZE, "%d", connected_with_index);
    if(send_message2(server_queue_id, m) == -1){
        perror("msgsnd disconnect_handle");
        exit(EXIT_FAILURE);
    }
    mq_close(connected_with_client);
    printf("You left the chat.\n");
    connected_with_client = -1;
    connected_with_index = -1;
}

void stop_handler(){
    Message m;
    m.type = STOP;
    m.index = client_index;
    snprintf(&m.text[0], MESSAGE_SIZE, "%d", connected_with_index);
    if(send_message2(server_queue_id, &m) == -1){
        perror("msgsnd stop_handler");
        exit(EXIT_FAILURE);
    }
}

void disconnect(){
    printf("Your friend left the chat.\n");
    connected_with_client = -1;
    connected_with_index = -1;
}

void parse_and_send(Message *m, char *buf){
    if(starts_with(buf, "STOP") || starts_with(buf, "stop")){ //this really should be equal, but...
        cleanup();
    }

    if(connected_with_index == -1){
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
            m->type = TEXT;
            strncpy(&m->text[0], &buf[0], MESSAGE_SIZE);
            if(send_message2(connected_with_client, m) == -1){
                perror("msgsnd sending to client");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void listen(){
    Message message;
    char buf[512];
    char *ptr;

    for(;;){
        if(connected_with_index == -1){
            //fgets is blocking
            if(fgets(&buf[0], sizeof(buf), stdin) != NULL){
                message.index = client_index;
                message.text[0] = '\0';
                parse_and_send(&message, &buf[0]);
                memset(&message, 0, sizeof(Message));
            }
        }else{
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
    //notify server to close
    stop_handler();

    mq_close(server_queue_id);
    mq_close(client_queue_id);
    mq_unlink(fname);
    _Exit(EXIT_SUCCESS);
}

void process_message(union sigval sv){
    register_event();
    Message m;
    if(get_message(client_queue_id, &m) == -1){
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
        case LIST:{
            printf("%s", m.text);
            fflush(stdout);
            break;
        }
        case TEXT:{
            printf("%50s", m.text);
            fflush(stdout);
            break;
        }
        default:{

        }
    }
}

void register_event() {
    struct sigevent e;

    e.sigev_notify_function = process_message;
    e.sigev_notify = SIGEV_THREAD;
    e.sigev_value.sival_ptr = NULL;
    e.sigev_notify_attributes = NULL;

    mq_notify(client_queue_id, &e);
}

int main(int argc, char *argv[]){
    signal(SIGINT, cleanup);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MESSAGE_LEN;
    attr.mq_curmsgs = 0;


    sprintf(&fname[0], "/client_%d", getpid());
    client_queue_id = mq_open(&fname[0], O_RDWR | O_CREAT, PERMISSIONS, &attr);
    printf("Opened queue %s %d\n", fname, client_queue_id);

    connected_with_client = -1;
    connected_with_index = -1;
    client_index = initialize_connection();
    printf("My ID: %d\n", client_index);

    register_event();

    listen();

    return 1;
}
