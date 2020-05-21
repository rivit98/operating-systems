#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <poll.h>

#include "common.h"


int network_socket_fd = -1;
int local_socket_fd = -1;

typedef struct{
    char *name;
    int sock_fd;
    int opp_index;
    kStatus status;
} Client;
Client clients[MAX_CLIENTS];


pthread_mutex_t client_array_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_POLL (MAX_CLIENTS + 2)
struct pollfd fds[MAX_POLL];

int accept2(){
    memset(&fds[0], 0, MAX_POLL * sizeof(struct pollfd));
    fds[0].fd = network_socket_fd;
    fds[0].events = POLLIN;
    fds[1].fd = local_socket_fd;
    fds[1].events = POLLIN;

    pthread_mutex_lock(&client_array_mutex);
    for(int i = 0; i < MAX_CLIENTS; i++){
//        if(clients[i].status == NOT_CONNECTED){
//            continue;
//        }
        fds[i + 2].fd = clients[i].sock_fd;
        fds[i + 2].events = POLLIN;
    }
    pthread_mutex_unlock(&client_array_mutex);

    poll(fds, MAX_POLL, -1);

    int fd_ret = -1;
    for(int i = 0; i < MAX_POLL; i++) {
        if(fds[i].revents & POLLIN){
            fd_ret = fds[i].fd;
            break;
        }
    }

    if(fd_ret == network_socket_fd || fd_ret == local_socket_fd){
        fd_ret = accept(fd_ret, NULL, NULL); // create new socket for client data
    }
    return fd_ret;
}

void signal_handler(){
    puts("SIGNAL HANDLER");
    exit(0);
}

void cleanup(void){
    if(network_socket_fd != -1){
        shutdown(network_socket_fd, SHUT_RDWR);
        close(network_socket_fd);
    }

    if(local_socket_fd != -1){
        shutdown(local_socket_fd, SHUT_RDWR);
        close(local_socket_fd);
    }

    for(int i = 0; i < MAX_CLIENTS; i++){
        free(clients[i].name);
    }

    printf("cleanup\n");
    exit(0);
}

void init_local_socket(const char *path){
    struct sockaddr_un serveraddr;

    local_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, path);

    unlink(path);

    bind(local_socket_fd, (const struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
}

void init_network_socket(const char * port){
    struct sockaddr_in desc;

    desc.sin_family = AF_INET;
    desc.sin_addr.s_addr = inet_addr("0.0.0.0");
    desc.sin_port = htons(atoi(port));

    network_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(network_socket_fd == -1){
        perror("Init socket");
        exit(EXIT_FAILURE);
    }

    if(bind(network_socket_fd,  (struct sockaddr *)&desc, sizeof(struct sockaddr)) == -1){
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void init_sockets(const char * port, const char *path){
    init_local_socket(path);
    init_network_socket(port);

    if(network_socket_fd != -1){
        listen(network_socket_fd, MAX_CONNECTIONS);
    }

    if(local_socket_fd != -1){
        listen(local_socket_fd, MAX_CONNECTIONS);
    }

    if(local_socket_fd == -1 && network_socket_fd == -1){
        printf("init_socket failed\n");
        exit(EXIT_FAILURE);
    }
}

int get_free_slot(){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].status == NOT_CONNECTED){
            return i;
        }
    }

    return -1;
}

int get_client_by_name(const char *name){
    if(name == NULL){
        return -1;
    }

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].status == CONNECTED && clients[i].name != NULL && strcmp(clients[i].name, name) == 0){
            return i;
        }
    }

    return -1;
}

void send_message(int fd, kCommand c, const char* arg){
    char msg[MAX_MESSAGE_LEN];
    snprintf(msg, MAX_MESSAGE_LEN, "%d|%s", c, arg);
    send(fd, msg, MAX_MESSAGE_LEN, 0);
}

int main(int argc, char **argv){
    if(argc != 3){
        puts("server port socket_name");
        return 1;
    }

    atexit(cleanup);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    char *port = argv[1];
    char *socket_path = argv[2];

    init_sockets(port, socket_path);
    printf("SERVER STARTED\n");

    memset(clients, 0, sizeof(Client) * MAX_CLIENTS);
    while(1){
        int remote_fd = accept2();
        if(remote_fd == -1){
            continue;
        }

        char buffer[MAX_MESSAGE_LEN];
        if(recv(remote_fd, buffer, MAX_MESSAGE_LEN, 0) == 0){
            continue;
        }
        printf("recv: %s\n", buffer);

        kCommand cmd = atoi(strtok(&buffer[0], "|"));
        const char *name = strtok(NULL, "|");
        int cid = get_client_by_name(name);

        pthread_mutex_lock(&client_array_mutex);
        switch (cmd) {
            case REGISTER_CLIENT:{
                if(cid != -1){
                    break;
                }

                int idx = get_free_slot();
                if(idx != -1){
                    printf("REGISTER_CLIENT: %s, idx=%d, fd=%d\n", name, idx, remote_fd);
                    clients[idx].status = CONNECTED;
                    clients[idx].name = calloc(MAX_MESSAGE_LEN, sizeof(char));
                    clients[idx].sock_fd = remote_fd;
                    clients[idx].opp_index = NO_OPP;
                    strncpy(clients[idx].name, name, MAX_MESSAGE_LEN);
                }else{
                    send_message(remote_fd, NO_FREE_SLOTS, "");
                }


                break;
            }
        }
        pthread_mutex_unlock(&client_array_mutex);
    }

    return 0;
}