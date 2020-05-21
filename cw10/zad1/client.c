#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "common.h"


int remote_socket = -1;

void init_socket(const char *type, const char *dest){
    if(type[0] == 'l'){ //local
        struct sockaddr_un serveraddr;

        remote_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sun_family = AF_UNIX;
        strcpy(serveraddr.sun_path, dest);

        connect(remote_socket, (const struct sockaddr *) &serveraddr, SUN_LEN(&serveraddr));
    }else{ //network
        char ip_s[32] = {};
        char port_s[16] = {};
        char *colonPos = strchr(dest, ':');
        strncpy(port_s, colonPos + 1, sizeof(port_s));
        strncpy(ip_s, dest, (int)(colonPos - dest));

        unsigned short port = (unsigned short)atoi(port_s);
        unsigned int ip = (unsigned int)inet_addr(ip_s);
        if(ip == INADDR_NONE){
            printf("ip conv\n");
            exit(EXIT_FAILURE);
        }

        remote_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(remote_socket == -1){
            perror("Init socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in sock_info;
        sock_info.sin_family = AF_INET;
        sock_info.sin_addr.s_addr = ip;
        sock_info.sin_port = htons(port);

        int ret = connect(remote_socket, (struct sockaddr*)&sock_info, sizeof(struct sockaddr));
        if(ret == -1){
            perror("connect socket");
            exit(EXIT_FAILURE);
        }
    }
}

void signal_handler(){
    puts("SIGNAL HANDLER\n");
    exit(0);
}

void cleanup(void){
    if(remote_socket != -1){
        shutdown(remote_socket, SHUT_RDWR);
        close(remote_socket);
    }

    printf("cleanup\n");
    exit(0);
}

int main(int argc, char **argv){
    if(argc != 4){
        puts("client name connection_type [ip|socket_name]");
        return 1;
    }

    atexit(cleanup);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    char buffer[MAX_MESSAGE_LEN];
    char *name = argv[1];
    char *type = argv[2];
    char *dest = argv[3];

    init_socket(type, dest);
    sprintf(buffer, "%d|%s||", REGISTER_CLIENT, name);
    send(remote_socket, buffer, MAX_MESSAGE_LEN, 0);

    while(1){
        puts("sending");
        sprintf(buffer, "%d|%s||", 13, name);
        send(remote_socket, buffer, 32, 0);

        sleep(2);
    }

    return 0;
}
